#pragma once

#include <vector>

template<typename T>
class matrix {
	int rows_;
	int cols_;
	std::vector<T> data_;
public:
	matrix(int r = 0, int c = 0) : rows_(r), cols_(c), data_(r* c) {}

	T& operator()(int r, int c) {
		return data_[r * cols_ + c];
	}
	const T& operator() (int r, int c) const {
		return data_[r * cols_ + c];
	}
	int rows() const { return rows_; }
	int cols() const { return cols_; }
	int size() const { return rows_ * cols_; }
	int rawsize() const { return rows_ * cols_ * sizeof(T); }

	void resize(int r, int c) {
		rows_ = r;
		cols_ = c;
		data_.resize(r, c);
	}

	char* rawdata() { return reinterpret_cast<char*>(data_.data()); } //data_.data equivale a &data_[0]
	const char* rawdata() const { return reinterpret_cast<const char*>(data_.data()); }

	auto& data() { return data_; }
	const auto& data() const { return data_; }

	auto begin() { return data_.begin(); }
	auto begin() const { return data_.begin(); }
	auto end() { return data_.end(); }
	auto end() const { return data_.end(); }

	T& operator[](int i) {
		return data_[i];
	}

	const T& operator[] (int i) const {
		return data_[i];
	}

	bool empty() const {
		return rows_ == 0 && cols_ == 0;
	}
};