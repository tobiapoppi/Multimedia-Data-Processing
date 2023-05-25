#pragma once

#include <vector>

template<typename T>
class mat {
	uint32_t r_;
	uint32_t c_;
	std::vector<T> data_;

public:
	mat() : r_(0), c_(0), data_(0) {}
	mat(int r, int c) : r_(r), c_(c), data_(r*c*) {}

	void resize(int r, int c) {
		r_ = r;
		c_ = c;
		data_.resize(r, c);
	}

	int rows() { return r_; }
	int rows() const { return r_; }
	int cols() { return c_; }
	int cols() const { return c_; }
	int size() { return data_.size(); }
	int size() const { return data_.size(); }

	auto& begin() { return data_.begin(); }
	const auto& begin() const { return data_.begin(); }
	auto& end() { return data_.end(); }
	const auto& end() const { return data_.end(); }

	T& operator(int r, int c) { return data_[(r * c_) + c]; }
	const T& operator(int r, int c) const { return data_[(r * c_) + c]; }

	auto rawdata() { return data_.data(); }
	int rawsize() { return r_ * c_ * sizeof(T); }

};