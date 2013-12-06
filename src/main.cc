/*
 * main.cc
 *
 *  Created on: Jan 10, 2013
 *      Author: liuyi
 */

#include <cstdlib>

#include <opencv2/core/core.hpp>

#include "image/mser.h"
#include "test/test.h"

using namespace cv;

int main(int argc, char** argv) {
	Mat gray(Size(200, 200), CV_8UC1, Scalar(0));
	Point vetex1[4] = {Point(50, 50), Point(150, 50), Point(150, 150), Point(50, 150)};
	Point vetex2[4] = {Point(70, 70), Point(80, 70), Point(80, 80), Point(70, 80)};
	Point vetex3[4] = {Point(100, 100), Point(120, 100), Point(120, 120), Point(100, 120)};
	fillConvexPoly(gray, vetex1, 4, Scalar(1));
	fillConvexPoly(gray, vetex2, 4, Scalar(2));
	fillConvexPoly(gray, vetex3, 4, Scalar(2));
	TestUtils::ShowImage(gray*80);
	RegionTree<Region> region_tree(gray, 4, true);
	region_tree.Parse();
	Region* root = region_tree.GetRoot();
	Region* region = root->children().back();
	region = region->children().back();
	Mat bin;
	region->BuildMask(&bin);
	TestUtils::ShowImage(bin);

	return EXIT_SUCCESS;
}


