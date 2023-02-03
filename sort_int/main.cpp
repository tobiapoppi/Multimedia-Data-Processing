#include <cstdlib>
#include <cstdio>

#include "vector.h"


double mean(const mdp::vector<int>& v) {
	double s = 0.0;
	for (size_t i = 0; i < v.size(); ++i) {
		s += v[i];
	}
	return s/v.size();
}

void double_all_elements(mdp::vector<int>& v) {
	for (size_t i = 0; i < v.size(); ++i) {
		v[i] *= 2;
	}
}

int main(int argc, char* argv[]) {

	if (argc != 3) {
		return EXIT_FAILURE;
	}

	mdp::vector<int> v = mdp::read(argv[1]);
	
	if (v.empty()) {
		return EXIT_FAILURE;
	}

	mdp::sort(v);

	mdp::write(argv[2], v);

	return EXIT_SUCCESS;
}