#ifndef VECTOR_H
#define VECTOR_H

#include <cassert>
#include <utility>
#include <algorithm>

namespace mdp {

	template<typename T>
	class vector {
	private:
		size_t size_;
		size_t capacity_;
		T* data_;

	public:
		vector(size_t starting_size = 0) : size_(starting_size), capacity_(starting_size) {
			data_ = new T(capacity_);
		}
		vector(const vector& other) : size_(other.size_), capacity_(other.capacity_) {
			data_ = new T[capacity_];
			std::copy(other.data_, other.data_ + size_, data_);
		}
		vector(vector&& other) : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
			other.data_ = nullptr;
		}
		vector& operator=(vector rhs) {
			swap(*this, rhs);
			return *this;
		}

		~vector() {
			delete[] data_;
		}

		void resize(size_t new_size, size_t new_capacity = 0, bool force_capacity = false) {
			if (new_capacity == 0 || new_capacity < new_size) {
				new_capacity = new_size;
			}
			if (new_size > capacity_ || force_capacity) {
				capacity_ = new_capacity;
				auto* tmp = new T[capacity_];
				std::copy(data_, data_ + size_, tmp);
				delete[] data_;
				data_ = tmp;
			}
			size_ = new_size;
		}

		void push_back(const T& val) {
			resize(size_ + 1, capacity_ * 2 + 1);
			data_[size_ - 1] = val;
		}

		void shrink_to_fit() {
			resize(size_, size_, true);
		}

		size_t size() const {	//How do I tell the compiler that this method ensure me not to modify the object? with const. But in c++ I can't write vector* this. Quindi metto solo const fuori dalle parentesi.
			return size_;
		}
		auto* data() {
			return data_;
		}
		const auto& operator[](size_t pos) const {
			return data_[pos];
		}
		auto& operator[](size_t pos) {
			return data_[pos];
		}
		const auto& at(size_t pos) const {
			assert(pos < size_);
			return data_[pos];
		}
		auto& at(size_t pos) {
			assert(pos < size_);
			return data_[pos];
		}
		bool empty() const {
			return size_ == 0;
		}

		friend void swap(vector& a, vector& b);
	};

	template<typename T>
	int compare(const void* va, const void* vb) {
		const auto* a = (const T*)va;
		const auto* b = (const T*)vb;

		if (*a < *b) {
			return -1;
		}
		else if (*b < *a) {
			return 1;
		}
		return 0;
	}

	template<typename T>
	void sort(vector<T>& v) {
		qsort(v.data(), v.size(), sizeof(T), compare<T>);
	}

	bool write(const char* filename, vector<int>& v);

	vector<int> read(const char* filename);

}
#endif
