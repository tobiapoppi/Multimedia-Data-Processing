#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <istream>
#include "mat.h"
#include "ppm.h"
#include "ppm.cpp"

uint8_t Base64Table(uint8_t x) {
	if (x >= 0 && x <= 25) {
		return x + 65;
	}
	else if (x >= 26 && x <= 51) {
		return x + 71;
	}
	else if (x >= 52 && x <= 61) {
		return x - 4;
	}
	else if (x == 62) {
		return 43;
	}
	else {
		return 47;
	}
}

std::string Base64Encode(const std::vector<uint8_t>& v) {
	std::vector<uint8_t> v_c(v.size());
	std::copy(v.begin(), v.end(), v_c.begin());
	std::string s;
	while (v_c.size() % 3 != 0) {
		v_c.push_back(128);
	}
	for (size_t i = 0; i < v_c.size(); i += 3) {
		uint32_t bit24;
		bit24 = (v_c[i] << 16) + (v_c[i + 1] << 8) + (v_c[i + 2]);
		uint8_t a = (bit24 & 0b00000000111111000000000000000000) >> 18;
		uint8_t b = (bit24 & 0b00000000000000111111000000000000) >> 12;
		uint8_t c = (bit24 & 0b00000000000000000000111111000000) >> 6;
		uint8_t d = (bit24 & 0b00000000000000000000000000111111);
		s.push_back(Base64Table(a));
		s.push_back(Base64Table(b));
		s.push_back(Base64Table(c));
		s.push_back(Base64Table(d));
	}
	return s;
}


void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	uint8_t curr;
	uint8_t next;
	uint8_t next_next;
	uint8_t copy_count;
	uint8_t run_count;
	std::vector<uint8_t> lit;
	size_t n = 0;
	bool end = false;

	while (true) {
		copy_count = 0;
		run_count = 0;
		lit.clear();
		curr = img.data()[n];

		//if I'm on the last position of input, I must end with a single literal
		if (n + 1 == img.size()) {
			encoded.push_back(0);
			encoded.push_back(curr);
			break;
		}

		next = img.data()[n + 1];

		//if the next position is the last one of input, I must end with a 2 literals of a 2-run.
		if (n + 2 == img.size()) {
			if (curr == next) {
				encoded.push_back(255);
				encoded.push_back(curr);
			}
			else {
				encoded.push_back(1);
				encoded.push_back(curr);
				encoded.push_back(next);
			}
			break;
		}

		if (curr != next) {
			//copy command
			lit.push_back(curr);
			++copy_count;
			next_next = img.data()[n + 2];

			while (curr != next && next != next_next && copy_count < 128) {
				curr = next;
				lit.push_back(curr);
				++n;
				++copy_count;
				next = next_next;

				if (n + 2 == img.size()) {
					//here I should increment copy_count to include next but I send directly copy_count instead of copy_count-1
					encoded.push_back(copy_count);
					for (size_t i = 0; i < lit.size(); ++i) {
						encoded.push_back(lit[i]);
					}
					encoded.push_back(next);
					end = true;
					break;
				}

				next_next = img.data()[n + 2];
			}
			if (end == true) {
				break;
			}
			encoded.push_back(copy_count - 1);
			for (size_t i = 0; i < lit.size(); ++i) {
				encoded.push_back(lit[i]);
			}
			++n;

		}
		else {
			//run command
			++run_count;
			while (curr == next && run_count <= 127) {
				++run_count;
				curr = next;
				++n;
				if (n + 1 == img.size()) {
					encoded.push_back(257 - run_count);
					encoded.push_back(curr);
					end = true;
					break;
				}
				next = img.data()[n + 1];
			}
			if (end == true) {
				break;
			}
			encoded.push_back(257 - run_count);
			encoded.push_back(curr);
			++n;
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


std::string JSON(const std::string& filename) {
	std::ifstream is(filename, std::ios::binary);
	std::string s;
	if (!is) {
		s = "{}";
		return s;
	}
	bool  r;
	mat<vec3b> img;
	r = LoadPPM(filename, img);
	if (!r) {
		s = "{}";
		return s;
	}
	mat<uint8_t> img_r;
	mat<uint8_t> img_g;
	mat<uint8_t> img_b;

	SplitRGB(img, img_r, img_g, img_b);

	std::vector<uint8_t> p_r;
	std::vector<uint8_t> p_g;
	std::vector<uint8_t> p_b;

	PackBitsEncode(img_r, p_r);
	PackBitsEncode(img_r, p_g);
	PackBitsEncode(img_r, p_b);

	std::string s_r;
	std::string s_g;
	std::string s_b;
	s_r = Base64Encode(p_r);
	s_g = Base64Encode(p_g);
	s_b = Base64Encode(p_b);
	s.insert(0, "{\"width\": ");
	s.insert(s.size(), std::to_string(img.cols()));
	s.insert(s.size(), ",\n\"height\": ");
	s.insert(s.size(), std::to_string(img.rows()));
	s.insert(s.size(), ",\n\"red\": \"");
	s.insert(s.size(), s_r);
	s.insert(s.size(), "\",\n\"green\": \"");
	s.insert(s.size(), s_g);
	s.insert(s.size(), "\",\n\"blue\": \"");
	s.insert(s.size(), s_b);
	s.insert(s.size(), "\"\n}");
	return s;
}

/*
int main(int argc, char** argv) {
	std::string s;
	s = JSON(argv[1]);
}
*/