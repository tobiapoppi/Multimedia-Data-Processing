#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>

#include "mat.h"
#include "ppm.h"

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
	//std::copy_n(std::istream_iterator<uint8_t>(is), img.rawsize(), img.rawdata());
	return true;
}

/*
int main(int argc, char** argv) {
	bool r;
	mat<vec3b> img;
	r = LoadPPM(argv[1], img);
}*/