#include "pgm.h"
#include <fstream>
#include <string>

bool write(const std::string& filename, const matrix<uint8_t>& im, pgm_mode mode) {

	std::ofstream os(filename, std::ios::binary);

	if (!os) {
		std::logic_error("Cannot open input file.\n");
		return false;
	}
	os << "P" << int(mode) << '\n';
	os << "# MDP2023 \n";
	os << im.cols() << " " << im.rows() << "\n255\n";

	if (mode == pgm_mode::plain) {
		/*
		for (size_t r = 0; r < im.rows(); ++r) {
			for (size_t c = 0; c < im.cols(); ++c) {
				os << +im(r, c) << " ";
			}
			os << "\n";
		}*/

		// single loop version
		/*
		for (size_t i = 0; i < im.size(); ++i) {
			os << +im[i] << " ";
		}*/

		// iterator version
		for (auto it = im.begin(); it != im.end(); ++it) {
			os << +*it << ' ';
		}

		//since we im has begin and end, I can also use range-based for.

		//or, I can do alg. copy through iterators
		//copy(im.begin(), im.end(), std::ostream_iterator<int>(os, " "));
	}
	else {
		os.write(im.rawdata(), im.rawsize());
		/* exact same thing but slower
		for (size_t r = 0; r < im.rows(); ++r) {
			for (size_t c = 0; c < im.cols(); ++c) {
				os << im(r, c);
			}
		}*/
	}
}