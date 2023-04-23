#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>

#include "mat.h"
#include "ppm.h"
//#include "compress.cpp"


void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	uint8_t curr;
	uint8_t prev;
	bool init = true;
	bool literalling = false;
	bool running = false;
	uint8_t cp_count = 1;
	std::vector<uint8_t> lit;

	for (size_t i = 0; i < img.rows(); ++i) {
		for (size_t j = 0; j < img.cols(); ++j) {
			curr = img(i, j);

			if (init == true) {
				prev = curr;
				init = false;
			}
			else {
				if (running) {
					if (curr == prev) {
						if (cp_count == 128) {
							encoded.push_back(129);
							encoded.push_back(curr);
							cp_count = 1;
							running = true;
							literalling = false;
						}
						else {
							++cp_count;
						}
					}
					else {
						encoded.push_back(257 - cp_count);
						encoded.push_back(prev);
						running = false;
						literalling = true;
						prev = curr;
						cp_count = 1;
					}
				}
				else if (literalling) {
					if (curr != prev) {
						if (lit.size() < 128) {
							lit.push_back(prev);
							prev = curr;
						}
						else {
							encoded.push_back(lit.size() - 1);
							for (size_t k = 0; k < lit.size(); ++k) {
								encoded.push_back(lit[k]);
							}
							lit.clear();
							literalling = true;
							prev = curr;
						}
					}
					else {
						encoded.push_back(lit.size() - 1);
						for (size_t k = 0; k < lit.size(); ++k) {
							encoded.push_back(lit[k]);
						}
						lit.clear();
						literalling = false;
						running = true;
						prev = curr;
						++cp_count;
					}
				}
				else {
					if (curr == prev) {
						running = true;
						literalling = false;
						++cp_count;
					}
					else {
						literalling = true;
						running = false;
						lit.push_back(prev);
						prev = curr;
					}
				}
			}
		}
	}
	encoded.push_back(128);
}

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

	std::vector<uint8_t> p;

	PackBitsEncode(img_r, p);
}