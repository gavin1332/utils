// Copyright (c) 2010-2011, Tuji
// All rights reserved.
// 
// ${license}
//
// Author: LIU Yi

#include "utils/math/math.h"

#include <cmath>

using namespace std;
using namespace cv;

const double MathUtils::kPI = 3.1415926;

bool MathUtils::IsRectIntersected(Rect a, Rect b, Rect* intersect) {
  int x = std::max(a.x, b.x);
  int y = std::max(a.y, b.y);
  int width = std::min(a.x + a.width, b.x + b.width) - x;
  int height = std::min(a.y + a.height, b.y + b.height) - y;

  if (intersect != NULL) {
    *intersect = Rect(x, y, width, height);
  }

  return width > 0;
}

void MathUtils::CalcHessian(const Mat& mat, Point pos, Mat* hessian) {
  CV_Assert(mat.type() == CV_32FC1 && mat.rows >= 3 && mat.cols >= 3);

  static const int size = 3;
  static const Mat xx_filter = (Mat_<float>(1, size)
      << 1, -2, 1);
  static const Mat yy_filter = (Mat_<float>(size, 1)
      << 1, -2, 1);
  static const Mat xy_filter = (Mat_<float>(size, size)
      << 0.25f, 0, -0.25f, 0, 0, 0, -0.25f, 0, 0.25f);

  Point inner = pos;
    // Shift the point to the inner neighbor if the point is located on the border
  if (pos.x == 0) {
    inner.x = 1;
  } else if (pos.x == mat.cols - 1) {
    inner.x = mat.cols - 2;
  }

  if (pos.y == 0) {
    inner.y = 1;
  } else if (pos.y == mat.rows - 1) {
    inner.y = mat.rows - 2;
  }

  Mat roi = mat(Rect(inner.x - 1, inner.y, size, 1));
  hessian->at<float>(0, 0) = static_cast<float>(roi.dot(xx_filter));

  roi = mat(Rect(inner.x, inner.y - 1, 1, size));
  hessian->at<float>(1, 1) = static_cast<float>(roi.dot(yy_filter));

  roi = mat(Rect(inner.x - 1, inner.y - 1, size, size));
  hessian->at<float>(0, 1) = static_cast<float>(roi.dot(xy_filter));
  hessian->at<float>(1, 0) = hessian->at<float>(0, 1);
}
