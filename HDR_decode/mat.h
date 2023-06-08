#ifndef MAT_H
#define MAT_H

#include <cstdlib>
#include <cstdint>
#include <vector>


template<typename T>
struct mat {
	int r_;
	int c_;
	std::vector<T> data_;

	mat(int r, int c) : r_(r), c_(c), data_(r* c) {}
	void resize(int r, int c) {
		r_ = r;
		c_ = c;
		data_.resize(r*c);
	}

	int rows() { return r_; }
	const int rows() const { return r_; }
	int cols() { return c_; }
	const int cols() const { return c_; }

	T* rawdata() { return data_.data(); }
	const T* rawdata() const { return &data_.data(); }
	int rawsize() { return r_ * c_ * sizeof(T); }
	const int rawsize() const { return r_ * c_ * sizeof(T); }
	T& operator() (int r, int c) {
		return data_[(r * c_) + c];
	}
};

#endif // !1