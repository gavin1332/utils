// Copyright (c) 2010-2011, Tuji
// All rights reserved.
// 
// ${license}
//
// Author: LIU Yi

#ifndef UTILS_IMAGE_SCALE_SPACE_H_
#define UTILS_IMAGE_SCALE_SPACE_H_

#include <opencv2/core/core.hpp>

#include "utils/math/math.h"

class ScaleSpace {
public:
  struct Param {
    static const int kDefaultNOctaves = 4;
    static const int kDefaultNOctaveLayers = 3;
    static const int kDefaultFirstOctave = -1;
    static const int kDefaultFirstOctaveLayer = -1;
    static const double kDefaultSigma;

    const int n_octaves;
    const int n_octave_layers;
    const int first_octave;
    const int first_octave_layer;
    const double sigma;

    Param() : n_octaves(kDefaultNOctaves),
        n_octave_layers(kDefaultNOctaveLayers),
        first_octave(kDefaultFirstOctave),
        first_octave_layer(kDefaultFirstOctaveLayer),
        sigma(kDefaultSigma) {}
    Param(int n_octaves, int n_octave_layers, int first_octave,
        int first_octave_layer, double sigma) : n_octaves(n_octaves),
        n_octave_layers(n_octave_layers), first_octave(first_octave),
        first_octave_layer(first_octave_layer), sigma(sigma) {}
  };

  // Calculate the max response of the unit step function with DoG operator.
  //
  // @param k the constant multiple of neighbor space scales,
  //          DoG(x, y, sigma) = G(x, y, k*sigma) - G(x, y, sigma)
  static double DoGMaxResp(double k) {
    double extremum = 0.5
        * (erf(sqrt(log(k) / (k*k - 1))) - erf(k * sqrt(log(k) / (k*k - 1))));
    if (extremum < 0) extremum = 0 - extremum;
    return extremum;
  }

  // Calculate the max response offset of the unit step function with DoG
  // operator.
  //
  // @param sigma the scale
  // @param k the constant multiple of neighbor space scales,
  //          DoG(x, y, sigma) = G(x, y, k*sigma) - G(x, y, sigma)
  static double DoGMaxRespOffset(double sigma, double k) {
    return k*sigma*sqrt(2*log(k)/(k*k - 1));
  }

  // Calculate the appropriate scale giving max response offset.
  //
  // @param offset the max response offset of the unit step function with DoG
  // @param k the constant multiple of neighbor space scales,
  //          DoG(x, y, sigma) = G(x, y, k*sigma) - G(x, y, sigma)
  static double CalcScaleFromMaxResp(double offset, double k) {
    return sqrt((k*k - 1)*offset*offset/(2*k*k*log(k)));
  }

  ScaleSpace();
  ScaleSpace(const Param& param);
  ScaleSpace(int O, int S, int omin, int smin, double sigma);
  virtual ~ScaleSpace();

  int O() const {
    return O_;
  }
  int S() const {
    return S_;
  }
  int omin() const {
    return omin_;
  }
  int omax() const {
    return O_ + omin_;
  }
  int smin() const {
    return smin_;
  }
  int smax() const {
    return smax_;
  }

  double max_edge_response() const {
    return max_edge_response_;
  }

  double sigmak() const {
    return sigmak_;
  }

  double DoGMaxResp() {
    return DoGMaxResp(sigmak_);
  }

  double DoGMaxRespOffset(double sigma) {
    return DoGMaxRespOffset(sigma, sigmak_);
  }

  double CalcScaleFromMaxResp(double offset) {
    return CalcScaleFromMaxResp(offset, sigmak_);
  }

  void Build(const cv::Mat& img);

  void GenerateExtremaMap();

  double GetScaleFromIndex(double o, double s) const {
    return sigma0_ * pow(2.0, o + s / S_);
  }

  void RetrieveIndexFromScale(double sigma, int* o, int* s) {
    *o = static_cast<int>(floor(log(sigma / sigma0_) / log(2)));
    *s = static_cast<int>(round(S_ * log(sigma / sigma0_ / pow(2, *o)) / log(2)));
    if (*s == S_) {
      *o += 1;
      *s = 0;
    }
  }

  double GetScaleFromIndex(double s) const {
    return GetScaleFromIndex(0, s);
  }

  const cv::Mat& GetOctaveLevel(int o, int s) const {
    return *GetOctaveLevelPtr(o, s);
  }

  const cv::Mat& GetExtremaMap(int o, int s) const {
    CV_Assert(!extrema_map_[0][0].empty() && omin_ <= o && o < omin_ + O_
        && smin_ + 1 <= s && s <= smax_ - 2);
    return extrema_map_[o - omin_][s - smin_ - 1];
  }

 private:
  // Buffer
  cv::Mat** octave_;

  // Extrema map
  cv::Mat** extrema_map_;

  // Scale space parameters
  int O_;
  int S_;
  int omin_;
  int smin_;
  int smax_;

  double sigman_;
  double sigma_;
  double sigmak_;
  double sigma0_;

  double max_edge_response_;

  cv::Mat* GetOctaveLevelPtr(int o, int s) const {
    assert(omin_ <= o && o < omin_ + O_ && smin_ <= s && s <= smax_);
    return octave_[o - omin_] + (s - smin_);
  }

  void InitBuffer();

  void MakePyramidBase(const cv::Mat& fimg);

  void MakeOctaves();

  void Upsample(const cv::Mat& src, cv::Mat* dst, int factor);
  void Downsample(const cv::Mat& src, cv::Mat* dst, int factor);

  cv::Mat** GenerateDoG();
  void ReleaseDoG(cv::Mat** dog);

};

#endif
