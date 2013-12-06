// Copyright (c) 2010-2011, Tuji
// All rights reserved.
//
// ${license}
//
// Author: LIU Yi

#include "image/image.h"

#include <stack>

#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;


void ImgUtils::Gradient(const Mat& gray, Mat* dx, Mat* dy) {
  Mat kernel = (Mat_<float>(1, 3) << -0.5, 0, 0.5);
  filter2D(gray, *dx, CV_32F, kernel);
  filter2D(gray, *dy, CV_32F, kernel.t());
}

void ImgUtils::GradientMagDir(const Mat& gray, Mat* grad) {
  Mat dx, dy;
  Gradient(gray, &dx, &dy);

  sqrt(dx.mul(dx) + dy.mul(dy), grad[0]);
  divide(dy, dx + FLT_MIN, grad[1]);

  MatIterator_<float> it = grad[1].begin<float>();
  MatIterator_<float> end = grad[1].end<float>();
  for (; it != end; ++it) {
    *it = atan(*it)/MathUtils::kPI + 0.5f;
    if (*it < 0) {
      *it = 0;
    } else if(*it > 1) {
      *it = 1;
    }
  }
}

bool ImgUtils::IsBinary(const Mat& img) {
  CV_Assert(img.type() == CV_8UC1);

  Mat_<uchar>::const_iterator itr = img.begin<uchar>();
  Mat_<uchar>::const_iterator end = img.end<uchar>();
  int value[2] = {*itr, -1};
  for (; itr != end; ++itr) {
    if (*itr != value[0]) {
      if (value[1] == -1) {
        value[1] = *itr;
      } else if (*itr != value[1]) {
        return false;
      }
    }
  }

  return true;
}

void ImgUtils::BGR2HVIntegrated(const Mat& src, Mat* dst) {
  CV_Assert(dst != NULL && src.type() == CV_8UC3);

  dst->create(src.size(), CV_8UC1);

  Mat hsv;
  cvtColor(src, hsv, CV_BGR2HSV);

  Mat_<Vec3b>::const_iterator src_it = src.begin<Vec3b>();
  Mat_<Vec3b>::const_iterator src_end = src.end<Vec3b>();
  Mat_<Vec3b>::iterator hsv_it = hsv.begin<Vec3b>();
  MatIterator_<uchar> dst_it = dst->begin<uchar>();
  for(; src_it != src_end; ++src_it, ++hsv_it, ++dst_it) {
    if(IsChromatic(*src_it)) {
      // ((*hsv_it)[0] * 1.0f / 180 * 0.4f + 0.6f) * 256
      *dst_it = static_cast<uchar>((*hsv_it)[0] * 0.5689f + 153.6f);
    } else {
      // (*hsv_it)[2] * 1.0f / 256 * 0.4f) * 256
      *dst_it = static_cast<uchar>((*hsv_it)[2] * 0.4f);
    }
  }
}

void ImgUtils::SimpleMorph(const Mat& binary, Mat* dst, int op, int iteration) {
  Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
  morphologyEx(binary, *dst, op, element, Point(1, 1), iteration);
}

void ImgUtils::Thin(const Mat& binary, Mat* dst) {
  const uchar kFG = 255;
  const uchar kBG = 0;
  CV_Assert(dst != NULL && binary.type() == CV_8UC1);

  Mat target(binary.rows + 3, binary.cols + 3, CV_8UC1, Scalar(kBG));
  Mat roi = target(Rect(1, 1, binary.cols, binary.rows));
  binary.copyTo(roi);

  Mat label = target.clone();

  int step = target.step1();
  int tH = target.rows;
  bool hit = true;
  bool removable = false;
  uchar *offset = NULL;
  uchar *start = NULL;
  uchar *end = NULL;
  while (hit) {
    hit = false;

    for (int i = 1; i < tH - 2; ++i) {
      start = target.ptr(i, 1);
      end = target.ptr(i, step - 2);

      for (offset = start; offset < end; ++offset) {
        if (*offset == kFG) {
          removable = false;

          if (*(offset + step - 1) == kFG && *(offset + step) == kFG
              && *(offset + step + 1) == kFG && *(offset - step - 1) == kBG
              && *(offset - step) == kBG && *(offset - step + 1) == kBG) {

            removable = true;
          } else if (*(offset - 1 - step) == kFG && *(offset - 1) == kFG
              && *(offset - 1 + step) == kFG && *(offset + 1 - step) == kBG
              && *(offset + 1) == kBG && *(offset + 1 + step) == kBG) {

            removable = true;
          } else if (*(offset - step - 1) == kFG && *(offset - step) == kFG
              && *(offset - step + 1) == kFG && *(offset + step - 1) == kBG
              && *(offset + step) == kBG && *(offset + step + 1) == kBG) {

            removable = true;
          } else if (*(offset + 1 - step) == kFG && *(offset + 1) == kFG
              && *(offset + 1 + step) == kFG && *(offset - 1 - step) == kBG
              && *(offset - 1) == kBG && *(offset - 1 + step) == kBG) {

            removable = true;
          } else if (*(offset - 1) == kFG && *(offset + step) == kFG
              && *(offset + 1) == kBG && *(offset - step + 1) == kBG
              && *(offset - step) == kBG) {

            removable = true;
          } else if (*(offset - 1) == kFG && *(offset - step) == kFG
              && *(offset + 1) == kBG && *(offset + step + 1) == kBG
              && *(offset + step) == kBG) {

            removable = true;
          } else if (*(offset + 1) == kFG && *(offset - step) == kFG
              && *(offset - 1) == kBG && *(offset + step - 1) == kBG
              && *(offset + step) == kBG) {

            removable = true;
          } else if (*(offset + 1) == kFG && *(offset + step) == kFG
              && *(offset - 1) == kBG && *(offset - step - 1) == kBG
              && *(offset - step) == kBG) {

            removable = true;
          }

          if (removable) {
            if (*(offset - step) == kFG && *(offset - step + 2) == kBG
                && *(offset - 1) == kBG && *(offset + 1) == kFG
                && *(offset + 2) == kBG && *(offset + step) == kFG
                && *(offset + step + 2) == kBG) {

              removable = false;
            } else if (*(offset - step + 1) == kBG && *(offset - step + 2) == kBG
                && *(offset - 1) == kBG && *(offset + 1) == kFG
                && *(offset + 2) == kBG && *(offset + step + 1) == kFG) {

              removable = false;
            } else if (*(offset - step + 1) == kFG && *(offset - 1) == kBG
                && *(offset + 1) == kFG && *(offset + 2) == kBG
                && *(offset + step + 1) == kBG && *(offset + step + 2) == kBG) {

              removable = false;
            } else if (*(offset - step) == kBG && *(offset - 1) == kFG
                && *(offset + 1) == kFG && *(offset + step) == kFG
                && *(offset + step * 2 - 1) == kBG && *(offset + step * 2) == kBG
                && *(offset + step * 2 + 1) == kBG) {

              removable = false;
            } else if (*(offset - step) == kBG && *(offset - step + 2) == kBG
                && *(offset + step - 1) == kFG && *(offset + step) == kFG
                && *(offset + step + 1) == kBG && *(offset + step * 2) == kBG
                && *(offset + step * 2 + 1) == kBG) {

              removable = false;
            } else if (*(offset - step) == kBG && *(offset + step - 1) == kBG
                && *(offset + step) == kFG && *(offset + step + 1) == kFG
                && *(offset + step * 2 - 1) == kBG
                && *(offset + step * 2) == kBG) {

              removable = false;
            }

            if (removable) {
              *(label.data + static_cast<int>(offset - target.data)) = kBG;

              hit = true;
            }
          }
        }
      }
    }

    label.copyTo(target);
  }

  if(dst->empty()) {
    dst->create(binary.size(), binary.type());
  }
  roi.copyTo(*dst);
}

void ImgUtils::RemoveNoise(Mat* binary) {
  const uchar kBG = 0;

  Mat border;
  copyMakeBorder(*binary, border, 1, 1, 1, 1, BORDER_CONSTANT);

  int step = border.step1();
  for (int y = 0; y < binary->rows; ++y) {
    uchar* ptr = border.ptr(y + 1) + 1;
    for (int x = 0; x < binary->cols; ++x, ++ptr) {
      if (*ptr == kBG) continue;

      if (*(ptr-step-1) == kBG && *(ptr-step) == kBG && *(ptr-step+1) == kBG
          && *(ptr-1) == kBG && *(ptr+1) == kBG && *(ptr+step-1) == kBG
          && *(ptr+step) == kBG && *(ptr+step+1) == kBG) {
        binary->at<uchar>(y, x) = kBG;
      }
    }
  }
}

int ImgUtils::Surrounding(const Mat& gray, Point pos, uchar target,
    vector<Point>* shift_vec) {
  // ignore border pixels
  if (pos.x == 0 || pos.x == gray.cols - 1 || pos.y == 0 || pos.y == gray.rows - 1) {
    return 0;
  }

  const uchar* ptr = gray.ptr(pos.y) + pos.x;
  int step = gray.step1();
  int count = 0;
  // horizontal and vertical direction is prior
  if (*(ptr - step) == target) {
    ++count;
    if (shift_vec != NULL) shift_vec->push_back(Point(0, -1));
  }
  if (*(ptr - 1) == target) {
    ++count;
    if (shift_vec != NULL) shift_vec->push_back(Point(-1, 0));
  }
  if (*(ptr + 1) == target) {
    ++count;
    if (shift_vec != NULL) shift_vec->push_back(Point(1, 0));
  }
  if (*(ptr + step) == target) {
    ++count;
    if (shift_vec != NULL) shift_vec->push_back(Point(0, 1));
  }
  if (*(ptr - step - 1) == target) {
    ++count;
    if (shift_vec != NULL) shift_vec->push_back(Point(-1, -1));
  }
  if (*(ptr - step + 1) == target) {
    ++count;
    if (shift_vec != NULL) shift_vec->push_back(Point(1, -1));
  }
  if (*(ptr + step - 1) == target) {
    ++count;
    if (shift_vec != NULL) shift_vec->push_back(Point(-1, 1));
  }
  if (*(ptr + step + 1) == target) {
    ++count;
    if (shift_vec != NULL) shift_vec->push_back(Point(1, 1));
  }

  return count;
}

void ImgUtils::StretchHistogram(const Mat& gray, Mat* dst) {
  const uchar kFG = 255;

  if (dst != &gray) *dst = gray.clone();
  double min, max;
  cv::minMaxLoc(*dst, &min, &max);
  uchar diff = max - min;
  for (int y = 0; y < dst->rows; ++y) {
    uchar* ptr = dst->ptr(y);
    for (int x = 0; x < dst->cols; ++x, ++ptr) {
      *ptr = static_cast<uchar>((float) (*ptr - min) / diff * kFG);
    }
  }
}

void ImgUtils::FillRect(Mat* binary, const Rect& rect, uchar src, uchar dst) {
  for (int y = 0; y < rect.height; ++y) {
    uchar* ptr = binary->ptr<uchar>(rect.y + y) + rect.x;
    for (int x = 0; x < rect.width; ++x, ++ptr) {
      if (*ptr == src) *ptr = dst;
    }
  }
}
