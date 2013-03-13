// Copyright (c) 2010-2011, Tuji
// All rights reserved.
//
// ${license}
//
// Author: LIU Yi

#ifndef UTILS_IMAGE_IMAGE_H_
#define UTILS_IMAGE_IMAGE_H_

#include <cmath>

#include <opencv2/core/core.hpp>

#include "utils/math/math.h"

class ImgUtils {
 public:

  // Calculate the gradient of an image
  //
  // @param mat the input matrix with type CV_32FC1
  // @param dx
  // @param dy
  static void Gradient(const cv::Mat& gray, cv::Mat* dx, cv::Mat* dy);

  // Calculate the gradient magnitude and direction of an image
  //
  // @param mat the input matrix with type CV_32FC1
  // @param grad grad[0] stores magnitude, and grad[1] stores direction
  static void GradientMagDir(const cv::Mat& gray, cv::Mat* grad);

  // Check whether the input image is binarized
  static bool IsBinary(const cv::Mat& img);

  // Check whether the point lies inside the input image
  static bool IsInside(const cv::Mat& img, cv::Point pos) {
    return pos.x >= 0 && pos.x < img.cols && pos.y >= 0 && pos.y < img.rows;
  }

  // TODO: 未解决hue值0°与359°间断的问题
  // Convert RBG image to hue and intensity integrated one
  //
  // J. Lim, J. Park, and G. Medioni,
  // “Text segmentation in color images using tensor voting,”
  // Image and Vision Computing, vol. 25, May. 2007, pp. 671-685.
  static void BGR2HVIntegrated(const cv::Mat& src, cv::Mat* dst);

  // Simple morphology operation
  // @param op Type of morphological operation, one of the following:
  //      MORPH_OPEN opening
  //      MORPH_CLOSE closing
  //      MORPH_GRADIENT morphological gradient
  //      MORPH_TOPHAT "top hat"
  //      MORPH_BLACKHAT "black hat"
  static void SimpleMorph(const cv::Mat& binary, cv::Mat* dst, int op,
      int iteration = 1);

  // Extract the skeleton of image
  //
  // 冯星奎, 李林艳, 等. 一种新的指纹图象细化算法. 中国图像图形学报. 1999年10月
  static void Thin(const cv::Mat& binary, cv::Mat* dst);

  // Stretch the histogram to bright image
  static void StretchHistogram(const cv::Mat& gray, cv::Mat* dst);

  // Remove single noise pints
  static void RemoveNoise(cv::Mat* binary);

  // Replace the values in the rectangle of the binary image from src to dst.
  static void FillRect(cv::Mat* binary, const cv::Rect& rect, uchar src, uchar dst);

  // Report surrounding points at pos
  static int Surrounding(const cv::Mat& gray, cv::Point pos, uchar target,
      std::vector<cv::Point>* shift_vec = NULL);

  static int FloodFill(cv::Mat* img, cv::Point seed, int new_value,
      cv::Rect* rect = NULL, int lo_diff = 0, int up_diff = 0);

 private:
  static cv::Mat sobel_filter_x_;
  static cv::Mat sobel_filter_y_;

  static bool IsChromatic(const cv::Vec3b bgr) {
    const float threshold = 20;
    return (abs(bgr[2] - bgr[1]) + abs(bgr[1] - bgr[0]) + abs(bgr[2] - bgr[0]))
        * 1.0f / 3 > threshold;
  }

};

#endif
