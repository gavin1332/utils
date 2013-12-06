/*
 * mser.h
 *
 *  Created on: Jan 10, 2013
 *      Author: liuyi
 */

#ifndef IMAGE_MSER_H_
#define IMAGE_MSER_H_

#include <vector>
#include <list>
#include <stack>

#include <opencv2/core/core.hpp>

class Pixel {
 public:
	// rank_ == -1 means that this pixel has not been visited
	Pixel(const cv::Point& pos, int level) : pos_(pos), level_(level), rank_(-1),
			parent_(this) {}

	void set_parent(Pixel* parent) { parent_ = parent; }

	const cv::Point& pos() const { return pos_; }

	int level() const { return level_; }

	int rank() const { return rank_; }
	void IncRank() { ++rank_; }

	const std::vector<Pixel*>& neighbor() const { return neighbor_; }
	std::vector<Pixel*>* neighbor() { return &neighbor_; }

	Pixel* FindParent();

	void Union(Pixel* Y);

 private:
	cv::Point pos_;
	int level_;
	int rank_;
	Pixel* parent_;
	std::vector<Pixel*> neighbor_;

};

class Region {
 public:
	Region(int level) : level_(level), region_pool_index_(-1), x1_(INT_MAX),
			y1_(INT_MAX), x2_(0), y2_(0), parent_(NULL) {}
	virtual ~Region() {}

	int x1() const { return x1_; }
	int y1() const { return y1_; }
	int x2() const { return x2_; }
	int y2() const { return y2_; }

	int Width() const { return x2_ - x1_ + 1; }
  int Height() const { return y2_ - y1_ + 1; }
  cv::Rect ToCvRect() const { return cv::Rect(x1_, y1_, Width(), Height()); }

  int level() const { return level_; }

	int region_pool_index() const { return region_pool_index_; }
	void set_region_pool_index(int index) { region_pool_index_ = index; }

	Region* parent() const { return parent_; }
	const std::vector<Region*>& children() const { return children_; }
	void AssignParent(Region* region) {
		parent_ = region;
		region->children_.push_back(this);
	}

	const std::vector<Pixel*>& pix_vec() const { return pix_vec_; }

	cv::Point AnyPixelPos() const { return pix_vec_.front()->pos(); }

	bool IsLeaf() const { return children_.empty(); }
	bool IsRoot() const { return parent_ == NULL; }

	void AddPixel(Pixel* pixel) {
		pix_vec_.push_back(pixel);

		x1_ = std::min(x1_, pixel->pos().x);
		y1_ = std::min(y1_, pixel->pos().y);
		x2_ = std::max(x2_, pixel->pos().x);
		y2_ = std::max(y2_, pixel->pos().y);
	}

	void BuildMask(cv::Mat* mask);

 protected:
	static const uchar kFG = 255;
	static const uchar kBG = 0;

	int level_;
	int region_pool_index_;
	int x1_, y1_, x2_, y2_;
	Region* parent_;
	std::vector<Pixel*> pix_vec_;
	std::vector<Region*> children_;

  Region();

};


template<typename RegionClass>
class RegionTree {
 public:
	RegionTree(const cv::Mat& gray, int max_level, bool sort_level_region_y1) :
			kMaxLevel(max_level), gray_(gray), kWidth(gray.cols), kHeight(gray.rows),
			sort_level_region_y1_(sort_level_region_y1) {
		pixel_pool_ = new Pixel*[kWidth * kHeight];
		level_region_index_ = new int[kMaxLevel + 2];
	}
	~RegionTree();

	void Parse();

	std::vector<Region*>* region_pool() { return &region_pool_; }

	const cv::Mat& region_map() const { return region_map_; }

	Region* GetRoot() const { return region_pool_.back(); }

	void GetLevelRegionIterator(int level, std::vector<Region*>::iterator* begin,
  		std::vector<Region*>::iterator* end) {
		*begin = region_pool_.begin() + level_region_index_[kMaxLevel-level];
		*end = region_pool_.begin() + level_region_index_[kMaxLevel-level+1];
	}

 private:
	typedef std::vector<Region*>::iterator RegionVecItr;
	typedef std::vector<Pixel*>::iterator PixVecItr;
	typedef std::list<Pixel*>::iterator PixlistItr;

	static bool CompareY1(Region* r1, Region* r2) {
	 return r1->y1() < r2->y1();
	}

	const int kMaxLevel;

	const cv::Mat& gray_;
	const int kWidth;
	const int kHeight;

	Pixel** pixel_pool_;

	bool sort_level_region_y1_;

	std::vector<Region*> region_pool_;
	int* level_region_index_;

	cv::Mat region_map_;

	void BoxSort(const cv::Mat& gray, std::vector<Pixel*>* box);

	void InsertLevelPixels(std::vector<Pixel*>* box);

	void InsertPixel(Pixel* pixel);

	Pixel* PixelAt(cv::Point pos) const {
		return pixel_pool_[pos.y*kWidth + pos.x];
	}

	void Traverse(Pixel* pixel, Region* region);

	void RetrieveLevelRegions(std::vector<Pixel*>* box, std::list<Pixel*>* candidate,
			int level);

	void RetrieveRegionFromSeed(Pixel* seed, int level) {
		Region* region = new RegionClass(level);
		Traverse(seed, region);
		region_pool_.push_back(region);
	}

	void AssignRegionIndex();

	void BuildTree();

	void FillRegion(Region* region, int index);

	bool IsOutOfBorder(cv::Point pos) {
		return pos.x < 0 || pos.y < 0 || pos.x >= kWidth || pos.y >= kHeight;
	}

	bool IsVisited(cv::Point pos) {
		return PixelAt(pos)->rank() >= 0;
	}

	bool IsValid(cv::Point pos) {
		return !IsOutOfBorder(pos) && IsVisited(pos);
	}

};

template<typename RegionClass>
RegionTree<RegionClass>::~RegionTree() {
	int size = kWidth * kHeight;
	for (int i = 0; i < size; ++i) {
		delete pixel_pool_[i];
	}
	delete[] pixel_pool_;

	RegionVecItr it = region_pool_.begin(), end = region_pool_.end();
	for (; it != end; ++it) {
		delete *it;
	}
	region_pool_.clear();

	delete[] level_region_index_;
}

template<typename RegionClass>
void RegionTree<RegionClass>::BoxSort(const cv::Mat& gray, std::vector<Pixel*>* box) {
	int width = gray.cols;
	int height = gray.rows;
	int idx = 0;
	for (int y = 0; y < height; ++y) {
		const uchar* ptr = gray.ptr<uchar>(y);
		for (int x = 0; x < width; ++x, ++ptr, ++idx) {
			Pixel* pixel = new Pixel(cv::Point(x, y), *ptr);
			pixel_pool_[idx] = pixel;
			box[*ptr].push_back(pixel);
		}
	}
}

template<typename RegionClass>
void RegionTree<RegionClass>::InsertPixel(Pixel* pixel) {
	cv::Point pos = pixel->pos() + cv::Point(0, -1);
	if (IsValid(pos)) {
		pixel->Union(PixelAt(pos));
	}
	pos = pixel->pos() + cv::Point(-1, -1);
	if (IsValid(pos)) {
		pixel->Union(PixelAt(pos));
	}
	pos = pixel->pos() + cv::Point(1, -1);
	if (IsValid(pos)) {
		pixel->Union(PixelAt(pos));
	}
	pos = pixel->pos() + cv::Point(-1, 0);
	if (IsValid(pos)) {
		pixel->Union(PixelAt(pos));
	}
	pos = pixel->pos() + cv::Point(1, 0);
	if (IsValid(pos)) {
		pixel->Union(PixelAt(pos));
	}
	pos = pixel->pos() + cv::Point(0, 1);
	if (IsValid(pos)) {
		pixel->Union(PixelAt(pos));
	}
	pos = pixel->pos() + cv::Point(-1, 1);
	if (IsValid(pos)) {
    pixel->Union(PixelAt(pos));
	}
	pos = pixel->pos() + cv::Point(1, 1);
	if (IsValid(pos)) {
		pixel->Union(PixelAt(pos));
	}
	pixel->IncRank();
}

template<typename RegionClass>
void RegionTree<RegionClass>::Traverse(Pixel* pixel, Region* region) {
  std::stack<Pixel*> pixstack;
	std::stack<Pixel*> previous;
	pixstack.push(pixel);
	previous.push(NULL);
	while (!pixstack.empty()) {
		Pixel* pixel = pixstack.top();
		Pixel* pre = previous.top();
		pixstack.pop();
		previous.pop();
		region->AddPixel(pixel);

		std::vector<Pixel*>::iterator it = pixel->neighbor()->begin();
		std::vector<Pixel*>::iterator end = pixel->neighbor()->end();
		for (; it != end; ++it) {
			if (*it != pre) {
				pixstack.push(*it);
				previous.push(pixel);
			}
		}
	}
}

template<typename RegionClass>
void RegionTree<RegionClass>::FillRegion(Region* region, int index) {
	std::vector<Pixel*>::const_iterator it = region->pix_vec().begin();
	std::vector<Pixel*>::const_iterator end = region->pix_vec().end();
	for (; it != end; ++it) {
		region_map_.at<int32_t>((*it)->pos()) = index;
	}
}

template<typename RegionClass>
void RegionTree<RegionClass>::BuildTree() {
	region_map_ = cv::Mat(gray_.size(), CV_32SC1, cv::Scalar(region_pool_.size() - 1));
	for (int i = region_pool_.size() - 2; i >= 0; --i) {
		Region* region = region_pool_[i];
		cv::Point pos = region->AnyPixelPos();
		int index = region_map_.at<int32_t>(pos);
		region->AssignParent(region_pool_[index]);
		FillRegion(region, i);
	}
}

template<typename RegionClass>
void RegionTree<RegionClass>::InsertLevelPixels(std::vector<Pixel*>* box) {
	PixVecItr it = box->begin(), vend = box->end();
	for (; it != vend; ++it) {
		InsertPixel(*it);
	}
}

template<typename RegionClass>
void RegionTree<RegionClass>::RetrieveLevelRegions(std::vector<Pixel*>* box,
		std::list<Pixel*>* candidate, int level) {
	PixlistItr lit = candidate->begin(), lend = candidate->end();
	while (lit != lend) {
		if ((*lit)->FindParent() == *lit) {
			RetrieveRegionFromSeed(*lit, level);
			++lit;
		} else {
			lit = candidate->erase(lit);
		}
	}
	PixVecItr vit = box->begin(), vend = box->end();
	for (; vit != vend; ++vit) {
		if ((*vit)->FindParent() == *vit) {
			RetrieveRegionFromSeed(*vit, level);
			candidate->push_back(*vit);
		}
	}
}

template<typename RegionClass>
void RegionTree<RegionClass>::AssignRegionIndex() {
	if (sort_level_region_y1_) {
		std::vector<Region*>::iterator begin = region_pool_.begin();
		for (int level = 0; level < kMaxLevel; ++level) {
			sort(begin + level_region_index_[level],
					begin + level_region_index_[level+1], CompareY1);
		}
	}

	int size = region_pool_.size();
	for (int i = 0; i < size; ++i) {
		region_pool_[i]->set_region_pool_index(i);
	}
}

template<typename RegionClass>
void RegionTree<RegionClass>::Parse() {
	std::vector<Pixel*> box[kMaxLevel+1];
	BoxSort(gray_, box);

	std::list<Pixel*> candidate;
	level_region_index_[0] = 0;
	for (int level = kMaxLevel; level >= 0; --level) {
		InsertLevelPixels(box + level);
		RetrieveLevelRegions(box + level, &candidate, level);
		level_region_index_[kMaxLevel-level+1] = region_pool_.size();
	}
	AssignRegionIndex();

	BuildTree();
}


#endif
