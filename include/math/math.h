// Copyright (c) 2010-2011, Tuji
// All rights reserved.
// 
// ${license}
//
// Author: LIU Yi

#ifndef MATH_MATH_H_
#define MATH_MATH_H_

#include <cmath>
#include <ctime>

#include <vector>
#include <random>

#include <opencv2/core/core.hpp>

class Random {
 public:
  Random() {
    rng_.seed(static_cast<unsigned int>(time(NULL)));
  }
  Random(unsigned int seed) {
    rng_.seed(seed);
  }
  virtual ~Random() {}

 protected:
  std::mt19937 rng_;

  // OPTIMIZE
  virtual void abstract() = 0;

};

class RandomInt : public Random {
 public:
  RandomInt() {}
  RandomInt(unsigned int seed) : Random(seed) {}
  RandomInt(int min, int max_inclusive) : distribution_(min, max_inclusive) {}
  RandomInt(unsigned int seed, int min, int max_inclusive) : Random(seed),
      distribution_(min, max_inclusive) {}
  virtual ~RandomInt() {}

  int Next() {
    return distribution_(rng_);
  }

 private:
  std::uniform_int_distribution<int> distribution_;

  void abstract() {}

};

template <typename ResultType>
class RandomReal : public Random {
 public:
  RandomReal() {}
  RandomReal(unsigned int seed) : Random(seed) {}
  RandomReal(ResultType min, ResultType max_inclusive) : distribution_(min, max_inclusive) {}
  RandomReal(unsigned int seed, ResultType min, ResultType max_inclusive) : Random(seed),
      distribution_(min, max_inclusive) {}
  virtual ~RandomReal() {}

  ResultType Next() {
    return distribution_(rng_);
  }

 private:
  std::uniform_real_distribution<ResultType> distribution_;

  void abstract() {}
};

class MathUtils {
public:
  static const double kPI;

  static bool IsFloat0(float f) {
    return CmpFloat(f, 0) == 0;
  }

  static bool IsDouble0(double d) {
    return CmpDouble(d, 0) == 0;
  }

  static int CmpFloat(float a, float b) {
    if (fabs(a - b) <= 1e-6) {
      return 0;
    }

    return (a < b)? -1 : 1;
  }

  static int CmpDouble(double a, double b) {
    if (fabs(a - b) <= DBL_EPSILON) {
      return 0;
    }

    return (a < b)? -1 : 1;
  }

  template <typename T>
  class TravOpr {
   public:
    TravOpr() {}
    virtual ~TravOpr() {}

    virtual T operator() (T in) = 0;
  };

  template <typename T>
  static void MeanAndStdDev(const std::vector<T>& data_list, double* mean,
      double* std_dev);

  // Calculate the Hessian matrix of a given position of a matrix
  //
  // @param mat the input matrix with type CV_32FC1
  // @param pos the position to be calculated
  // @param hessian OUTPUT, the Hessian matrix
  static void CalcHessian(const cv::Mat& mat, cv::Point pos, cv::Mat* hessian);

  // Check whether the input two erect rectangles is intersected, and if true,
  // assign the parameter "intersect" with intersected rectangle
  static bool IsRectIntersected(cv::Rect a, cv::Rect b,
      cv::Rect* intersect = NULL);

  // Calculate the minimum bounding box
  static cv::Rect MinRectBoundingBox(cv::Rect a, cv::Rect b) {
    return cv::Rect(
        std::min(a.x, b.x),
        std::min(a.y, b.y),
        std::max(a.x + a.width, b.x + b.width) - std::min(a.x, b.x),
        std::max(a.y + a.height, b.y + b.height) - std::min(a.y, b.y)
        );
  }

  static double Point2LineDist(cv::Point p, cv::Vec3f line) {
    return abs(p.x*line[0] + p.y*line[1] + line[2]) / sqrt(line[0]*line[0] + line[1]*line[1]);
  }

};

template <typename T>
void MathUtils::MeanAndStdDev(const std::vector<T>& data_list, double* mean,
    double* std_dev) {
  size_t N = data_list.size();

  double sum = 0;
  typename std::vector<T>::const_iterator it = data_list.begin();
  for (; it != data_list.end(); ++it) {
    sum += *it;
  }
  *mean = sum / N;

  sum = 0;
  it = data_list.begin();
  for (; it != data_list.end(); ++it) {
    sum += (*it - *mean) * (*it - *mean);
  }
  *std_dev = sqrt(sum / N);
}

#endif
