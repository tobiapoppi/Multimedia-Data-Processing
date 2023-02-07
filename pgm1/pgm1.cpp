#include <cstdlib>
#include <string>
#include <fstream>

#include "matrix.h"

enum class pgm_mode {plain = 2, binary = 5};

bool write(const std::string& filename, const matrix<uint8_t>& im, pgm_mode mode) {
	
	std::ofstream os(filename);

	if (!os) {
		std::logic_error("Cannot open input file.\n");
		return false;
	}
	os << "P" << int(mode) << "\n";
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
		for (size_t i = 0; i < im.size(); ++i) {
			os << +im[i] << " ";
		}
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

int main(int argc, char** argv) {

	int r = 256;
	int c = 256;
	matrix<uint8_t> m(r, c);
	for (size_t i = 0; i < r; ++i) {
		for (size_t j = 0; j < r; ++j) {
			m(i, j) = i;
		}
	}

	write("test1.pgm", m, pgm_mode::plain);
	write("test1_bin.pgm", m, pgm_mode::binary);

	return EXIT_SUCCESS;
}