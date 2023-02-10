#include <cstdlib>
#include <fstream>
#include <string>
#include <iostream>
#include "pgm.h"


auto make_test_pattern() {
	int r = 256;
	int c = 256;
	matrix<uint8_t> m(r, c);
	for (size_t i = 0; i < r; ++i) {
		for (size_t j = 0; j < r; ++j) {
			m(i, j) = i;
		}
	}
	return m;
}

auto read(const std::string& filename) {
	std::ifstream is(filename, std::ios::binary);

	matrix<uint8_t> immagine;

	if (!is) {
		return immagine;
	}

	pgm_mode mode;

	std::string magic_number;
	std::getline(is, magic_number); //read everything until the end of the line (\n).
	
	if (magic_number == "P2") {
		mode = pgm_mode::plain;
	}
	else if (magic_number == "P5") {
		mode = pgm_mode::binary;
	}
	else {
		return immagine;
	}

	if (is.peek() == '#') {
		std::string comment;
		std::getline(is, comment);
	}

	int width, height, levels;
	char newline;
	is >> width >> height >> levels;
	is.get(newline);

	if (levels != 255 || newline != '\n') {
		return immagine;
	}

	immagine.resize(height, width);

	if (mode == pgm_mode::plain) {
		for(auto& x : immagine){
			int val;
			is >> val;
			x = val;
		}
	}
	else {
		is.read(immagine.rawdata(), immagine.rawsize());
	}

	return immagine;
}

void flip(matrix<uint8_t>& im) {
	for (size_t r = 0; r < im.rows()/2; ++r) {
		for (size_t c = 0; c < im.cols(); ++c) {
			std::swap(im(r, c), im(im.rows() - 1 - r, c));
		}
	}
}

int main(int argc, char** argv) {

	// solve exercise 1

	matrix<uint8_t> m = make_test_pattern();
	
	write("test1.pgm", m, pgm_mode::plain);
	write("test1_bin.pgm", m, pgm_mode::binary);

	//solve exercise 2
	m = read("test1_bin.pgm");
	if (m.empty()) {
		std::logic_error("Cannot read image.");
	}
	write("test___binout.pgm", m, pgm_mode::binary);
	return EXIT_SUCCESS;
}