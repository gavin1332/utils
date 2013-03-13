// Copyright (c) 2010-2011, Tuji
// All rights reserved.
// 
// ${license}
//
// Author: LIU Yi

#include "utils/image/scale_space.h"

#include <cmath>

#include <opencv2/imgproc/imgproc.hpp>

#include "utils/math/math.h"

using namespace std;
using namespace cv;

const double ScaleSpace::Param::kDefaultSigma = 1.6;

ScaleSpace::ScaleSpace() {
  ScaleSpace::Param param;
  new (this)ScaleSpace(param);
}

ScaleSpace::ScaleSpace(const Param& param) {
  new (this)ScaleSpace(param.n_octaves, param.n_octave_layers,
      param.first_octave, param.first_octave_layer,
      param.sigma);
}

ScaleSpace::ScaleSpace(int O, int S, int omin, int smin, double sigma) :
    O_(O), S_(S), omin_(omin), smin_(smin), sigma_(sigma) {
  smax_ = S_ + 1;
  sigman_ = 0.5;
  sigmak_ = pow(2.0, 1.0 / S_);
  sigma0_ = sigma_ * sigmak_;

  max_edge_response_ = DoGMaxResp(sigmak_);

  InitBuffer();
}

void ScaleSpace::InitBuffer() {
  octave_ = new Mat*[O_];
  extrema_map_ = new Mat*[O_];
  for (int i = 0; i < O_; ++i) {
    octave_[i] = new Mat[smax_ - smin_ + 1];
    extrema_map_[i] = new Mat[smax_ - smin_ - 2];
  }
}

ScaleSpace::~ScaleSpace() {
  if (octave_) {
    for (int i = 0; i < O_; ++i) {
      delete[] octave_[i];
    }
    delete[] octave_;
  }
  octave_ = NULL;

  if (extrema_map_) {
    for (int i = 0; i < O_; ++i) {
      delete[] extrema_map_[i];
    }
    delete[] extrema_map_;
  }
  extrema_map_ = NULL;
}

void ScaleSpace::Build(const Mat& img) {
  if (img.empty() || img.type() != CV_8UC1) {
    CV_Error(CV_StsBadArg, "img is empty or has incorrect type");
  }

  Mat fimg;
  img.convertTo(fimg, CV_32FC1, 1.0/255);

  MakePyramidBase(fimg);
  MakeOctaves();
}

void ScaleSpace::MakePyramidBase(const Mat& fimg) {
  if (omin_ < 0) {
    Upsample(fimg, octave_[0], 2);

    for (int o = -1; o > omin_; --o) {
      Upsample(octave_[0][0], octave_[0], 2);
    }
  } else if (omin_ > 0) {
    Downsample(fimg, octave_[0], 1 << omin_);
  } else {
    octave_[0][0] = fimg.clone();
  }

  double sa = sigma0_ * pow(sigmak_, smin_);
  double sb = sigman_ / pow(2.0, omin_); // review this
  if (sa > sb) {
    double sd = sqrt(sa*sa - sb*sb);
    GaussianBlur(octave_[0][0], octave_[0][0], Size(0, 0), sd);
  }
}

void ScaleSpace::MakeOctaves() {
  double dsigma0 = sigma0_ * sqrt(1.0 - 1.0/(sigmak_*sigmak_));

  for (int o = omin_; o < omin_ + O_; ++o) {
    // Prepare octave base
    if (o > omin_) {
      int sbest = min(smin_ + S_, smax_);
      Downsample(*GetOctaveLevelPtr(o - 1, sbest),
          GetOctaveLevelPtr(o, smin_), 2);

      double sa = sigma0_ * pow(sigmak_, smin_);
      double sb = sigma0_ * pow(sigmak_, sbest - S_);
      if (sa > sb) {
        double sd = sqrt(sa*sa - sb*sb);
        GaussianBlur(*GetOctaveLevelPtr(o, 0), *GetOctaveLevelPtr(o, 0),
            Size(0, 0), sd);
      }
    }

    // Make other levels
    for (int s = smin_ + 1; s <= smax_; ++s) {
      double sd = dsigma0 * pow(sigmak_, s);
      GaussianBlur(*GetOctaveLevelPtr(o, s - 1), *GetOctaveLevelPtr(o, s),
          Size(0, 0), sd);
    }
  }
}

void ScaleSpace::GenerateExtremaMap() {
  Mat** dog = GenerateDoG();
  for (int i = 0; i < O_; ++i) {
    for (int j = 0; j < smax_ - smin_ - 2; ++j) {
      extrema_map_[i][j].create(dog[i][j + 1].size(), CV_32FC1);

      MatIterator_<float> up_it = dog[i][j + 2].begin<float>();
      MatIterator_<float> mid_it = dog[i][j + 1].begin<float>();
      MatIterator_<float> down_it = dog[i][j].begin<float>();
      MatIterator_<float> ext_it = extrema_map_[i][j].begin<float>();
      MatIterator_<float> ext_end = extrema_map_[i][j].end<float>();

      for (; ext_it != ext_end; ++up_it, ++mid_it, ++down_it, ++ext_it) {
        if ((*up_it > *mid_it && *mid_it > *down_it)
            || (*down_it > *mid_it && *mid_it > *up_it)) {
          *ext_it = 0;
        } else {
          *ext_it = *mid_it;
        }
      }
    }
  }

  ReleaseDoG(dog);
}

Mat** ScaleSpace::GenerateDoG() {
  Mat** dog = new Mat*[O_];
  for (int i = 0; i < O_; ++i) {
    dog[i] = new Mat[smax_ - smin_];
  }

  for (int i = 0; i < O_; ++i) {
    for (int j = 0; j < smax_ - smin_; ++j) {
      dog[i][j] = octave_[i][j + 1] - octave_[i][j];
    }
  }

  return dog;
}

void ScaleSpace::ReleaseDoG(Mat** dog) {
  for (int i = 0; i < O_; ++i) {
    delete[] dog[i];
  }
  delete[] dog;
  dog = NULL;
}

void ScaleSpace::Upsample(const Mat& src, Mat* dst, int factor) {
  CV_Assert(factor > 0);

  resize(src, *dst, Size(0, 0), factor, factor, INTER_LINEAR);
}

void ScaleSpace::Downsample(const Mat& src, Mat* dst, int factor) {
  CV_Assert(factor > 0);

  double ratio = 1.0 / factor;
  resize(src, *dst, Size(0, 0), ratio, ratio, INTER_NEAREST);
}
