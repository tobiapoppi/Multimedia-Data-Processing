#include "vector.h"

#include <cstdio>
#include <utility>

namespace mdp {
	bool write(const char* filename, vector<int>& v) {
		FILE* fout = fopen(filename, "w");
		if (fout == NULL) {
			return false;
		}

		for (size_t i = 0; i < v.size(); ++i) {
			fprintf(fout, "%d\n", v[i]);
		}
		fclose(fout);
		return true;
	}

	vector<int> read(const char* filename) {

		vector<int> v;

		FILE* f = fopen(filename, "r");
		if (f == NULL) {
			return v;
		}

		int val;
		while (fscanf(f, "%d", &val) == 1) {
			v.push_back(val);
		}
		fclose(f);
		v.shrink_to_fit();
		return v;
	}

	void swap(vector<int>& a, vector<int>& b) {
		using std::swap;
		swap(a.size_, b.size_);
		swap(a.capacity_, b.capacity_);
		swap(a.data_, b.data_);
	}
}