#include <vector>
#include <algorithm>
#include <fstream>
#include <iterator>

std::vector<int> read(const char* filename) {
	std::vector<int> v;

	std::ifstream is(filename); //default mode for opening files is text mode. with the mode std::ios::binary specifichiamo la mode binary. 
	if (is) {
		int val;
		while (is >> val) { // >> IS USED ONLY FOR TEXT!!!
			v.push_back(val);
		}
		//is.close();   they are automatically closed at the end of scope.
		v.shrink_to_fit();
	}
	return v;
}

bool write(const char* filename, std::vector<int>& v) {
	std::ofstream os(filename);
	if (!os) {
		return false;
	}

	/*
	for (auto& x : v) {
		os << x << '\n';
	}*/

	copy(v.begin(), v.end(), std::ostream_iterator<int>(os, "\n"));

	return true;
}

int main(int argc, char* argv[]) {

	if (argc != 3) {
		return EXIT_FAILURE;
	}

	std::vector<int> v = read(argv[1]);

	std::ifstream is(argv[1]);
	std::istream_iterator<int> it_start = is;
	std::istream_iterator<int> it_stop;
	
	//std::for_each(it_start, it_stop, [&](int x) {v.push_back(x);});

	//std::copy(it_start, it_stop, std::back_inserter(v));

	std::vector<int> v{ std::istream_iterator<int>(is), std::istream_iterator<int>() };

	std::sort(v.begin(), v.end());

	write(argv[2], v);

	return EXIT_SUCCESS;
}