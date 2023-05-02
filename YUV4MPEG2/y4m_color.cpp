#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

#include "mat.h"
#include "pgm.h"
#include "ppm.h"
#include "types.h"
#include "utils.h"

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
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

		mat<vec3b> f_RGB(H, W);

		//read all Y
		mat<uint8_t> Y(H, W);
		is.read(reinterpret_cast<char*>(Y.rawdata()), Y.rawsize());

		//read all Cb
		mat<uint8_t> Cb_d(H/2, W/2);
		is.read(reinterpret_cast<char*>(Cb_d.rawdata()), Cb_d.rawsize());

		//read all Cr
		mat<uint8_t> Cr_d(H / 2, W / 2);
		is.read(reinterpret_cast<char*>(Cr_d.rawdata()), Cr_d.rawsize());


		//put everything inside the frame while converting to RGB
		for (size_t i = 0; i < f_RGB.rows(); ++i) {
			for (size_t j = 0; j < f_RGB.cols(); ++j) {
				int y = Y(i, j);
				int cb = Cb_d(i / 2, j / 2);
				int cr = Cr_d(i / 2, j / 2);

				y = y < 16 ? 16 : y > 235 ? 235 : y;
				cb = cb < 16 ? 16 : cb > 240 ? 240 : cb;
				cr = cr < 16 ? 16 : cr > 240 ? 240 : cr;

				y -= 16;
				cb -= 128;
				cr -= 128;

				int R = ((y * 1164) + (cr * 1596)) / 1000;
				int G = ((y * 1164) - (cb * 392) - (cr * 813)) / 1000;
				int B = ((y * 1164) + (cb * 2017)) / 1000;
				R = R < 0 ? 0 : R > 255 ? 255 : R;
				G = G < 0 ? 0 : G > 255 ? 255 : G;
				B = B < 0 ? 0 : B > 255 ? 255 : B;
				f_RGB(i, j)[0] = uint8_t(R);
				f_RGB(i, j)[1] = uint8_t(G);
				f_RGB(i, j)[2] = uint8_t(B);	
			}
		}

		frames.push_back(f_RGB);
	}

	return true;
}

/*
int main(int argc, char** argv) {
	std::vector<mat<vec3b>> frames;
	bool res;
	if (y4m_extract_color(argv[1], frames)) {
		for (size_t i = 0; i < frames.size(); ++i) {
			std::stringstream ss;
			ss << std::setfill('0');
			ss << "frame" << std::setw(3) << i << ".pgm";
			save_ppm(ss.str(), frames[i]);
		}
	}
}*/