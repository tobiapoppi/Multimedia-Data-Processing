#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <string>

#include "mat.h"
#include "pgm.h"
#include "ppm.h"
#include "types.h"
#include "utils.h"

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file" << std::endl;
		return false;
	}
	std::string mn;
	std::string tmp;
	uint32_t W;
	uint32_t H;
	is >> mn;
	while (is.peek() != 0x0A) {
		is >> tmp;
		uint8_t tag = tmp.front();
		tmp.erase(tmp.begin());
		if (tag == 'W') {
			W = std::stoi(tmp);
		}
		else if (tag == 'H') {
			H = std::stoi(tmp);
		}
		else if (tag == 'C') {
			if (tmp != "420jpeg") {
				std::cout << "Error: program only accepts 420jpeg chroma subsampling";
				return false;
			}
		}
		else if (tag == 'I') {
			if (tmp != "p") {
				std::cout << "Error: program only accepts progressive interlacing";
				return false;
			}
		}
		else if (tag == 'F') {
			continue;
		}
		else if (tag == 'A') {
			continue;
		}
		else if (tag == 'X') {
			continue;
		}
		else {
			return false;
		}
	}
	uint8_t consume;
	consume = is.get();

	while (is.peek() != EOF) {
		std::string t;
		std::getline(is, t, char(0x0A));

		mat<uint8_t> f(H, W);
		is.read(reinterpret_cast<char*>(f.rawdata()), f.rawsize());
		is.seekg(((H * W) / 4) * 2, std::ios_base::cur);
		frames.push_back(f);
	}
	
	return true;
}

/*
int main(int argc, char** argv) {
	std::vector<mat<uint8_t>> frames;
	bool res;
	res = y4m_extract_gray(argv[1], frames);
}*/