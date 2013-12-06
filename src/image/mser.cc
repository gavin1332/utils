/*
 * mser.cc
 *
 *  Created on: Jan 10, 2013
 *      Author: liuyi
 */

/*
 * core.cc
 *
 *  Created on: Sep 5, 2012
 *      Author: liuyi
 */

#include "image/mser.h"

#include <opencv2/imgproc/imgproc.hpp>

#include "math/math.h"

using namespace std;
using namespace cv;


Pixel* Pixel::FindParent() {
	Pixel* node = this;
	while (node != node->parent_) {
		node = node->parent_;
		parent_ = node->parent_;
	}
	return parent_;
}

void Pixel::Union(Pixel* Y) {
	Pixel* A = FindParent();
	Pixel* B = Y->FindParent();
	if (A != B) {
		neighbor_.push_back(Y);
		Y->neighbor()->push_back(this);

		if (A->rank() > B->rank()) {
			B->set_parent(A);
		}	else {
			A->set_parent(B);
		}
		if (A->rank() == B->rank()) { B->IncRank(); }
	}
}

void Region::BuildMask(cv::Mat* mask) {
	Rect rect = ToCvRect();
	*mask = Mat::zeros(Size(rect.width, rect.height), CV_8UC1);
	Point shift(-rect.x, -rect.y);
	vector<Pixel*>::iterator it = pix_vec_.begin(), end = pix_vec_.end();
	for (; it != end; ++it) {
		mask->at<uchar>((*it)->pos() + shift) = 255;
	}
}
