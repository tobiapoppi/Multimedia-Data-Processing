#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>

#include "mat.h"
#include "ppm.h"
#include "compress.cpp"


bool LoadPPM(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file." << std::endl;
		return false;
	}
	std::string magic;
	std::uint32_t width;
	std::uint32_t height;
	std::uint32_t maxval;
	std::getline(is, magic);
	if (is.peek() == '#') {
		std::string comment;
		std::getline(is, comment);
	}
	is >> width;
	is >> height;
	is >> maxval;
	{
		uint8_t consume;
		consume = is.get();
	}
	img.resize(height, width);
	is.read(img.rawdata(), img.rawsize());
	return true;
}

void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b) {
	img_r.resize(img.rows(), img.cols());
	img_g.resize(img.rows(), img.cols());
	img_b.resize(img.rows(), img.cols());
	for (size_t i = 0; i < img.rows(); ++i) {
		for (size_t j = 0; j < img.cols(); ++j) {
			img_r(i, j) = img(i, j)[0];
			img_g(i, j) = img(i, j)[1];
			img_b(i, j) = img(i, j)[2];
		}
	}
}

int main(int argc, char** argv) {
	bool r;
	mat<vec3b> img;
	r = LoadPPM(argv[1], img);

	mat<uint8_t> img_r;
	mat<uint8_t> img_g;
	mat<uint8_t> img_b;

	SplitRGB(img, img_r, img_g, img_b);

	/*
	std::ofstream os(argv[2]);

	for (size_t i = 0; i < img_r.size(); ++i) {
		os << +img_r.data()[i] << std::endl;
	}
	*/
	std::vector<uint8_t> p;

	PackBitsEncode(img_r, p);
	std::string s;
	s = Base64Encode(p);
}