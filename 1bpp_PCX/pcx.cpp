#include "pcx.h"

#include <fstream>
#include <iostream>


bool read_header(std::ifstream& is, int& xsize, int& ysize, int& totalbytes) {
	uint8_t man, v, enc, bpp;
	uint16_t winXmin, winYmin, winXmax, winYmax, vdpi, hdpi;
	// ignore 48 bytes
	uint8_t reserved, cplanes;
	uint16_t Bppline, paletteinfo, horscr, verscr;
	// ignore 54 bytes

	is.read(reinterpret_cast<char*>(&man), 1);
	is.read(reinterpret_cast<char*>(&v), 1);
	is.read(reinterpret_cast<char*>(&enc), 1);
	is.read(reinterpret_cast<char*>(&bpp), 1);
	is.read(reinterpret_cast<char*>(&winXmin), 2);
	is.read(reinterpret_cast<char*>(&winYmin), 2);
	is.read(reinterpret_cast<char*>(&winXmax), 2);
	is.read(reinterpret_cast<char*>(&winYmax), 2);
	is.read(reinterpret_cast<char*>(&vdpi), 2);
	is.read(reinterpret_cast<char*>(&hdpi), 2);
	is.ignore(48);
	is.read(reinterpret_cast<char*>(&reserved), 1);
	is.read(reinterpret_cast<char*>(&cplanes), 1);
	is.read(reinterpret_cast<char*>(&Bppline), 2);
	is.read(reinterpret_cast<char*>(&paletteinfo), 2);
	is.read(reinterpret_cast<char*>(&horscr), 2);
	is.read(reinterpret_cast<char*>(&verscr), 2);
	is.ignore(54);

	xsize = winXmax - winXmin + 1;
	ysize = winYmax - winYmin + 1;

	totalbytes = cplanes * Bppline;
	return true;
}


bool load_pcx(const std::string& filename, mat<uint8_t>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "Cannot open input file." << std::endl;
		return false;
	}
	
	std::vector<uint8_t> out;

	int xsize, ysize;
	int totalbytes;

	if (!read_header(is, xsize, ysize, totalbytes)) {
		return false;
	}

	img.resize(ysize, xsize);

	int subtotal = 0;
	int r = 0;
	int c = 0;
	while (ysize-- > 0) {
		while (subtotal < totalbytes) {
			uint8_t b = 0;
			is.read(reinterpret_cast<char*>(&b), 1);
			if ((b >> 6) == 3) {
				uint8_t run = b & 0b00111111;
				uint8_t d;
				is.read(reinterpret_cast<char*>(&d), 1);

				while (run-- > 0) {
					for (uint8_t i = 0; i < 8; ++i) {
						img[r * img.cols() + (subtotal * 8) + i] = ((d >> (8 - i - 1)) & 0x01) == 0 ? 0 : 255;
						//out.push_back(d);
					}
					subtotal += 1;
					if (subtotal == totalbytes) break;
				}
			}
			else {
				size_t i = 8;
				for (uint8_t i = 0; i < 8; ++i) {
					img[r * img.cols() + (subtotal * 8) + i] = ((b >> (8 - i - 1)) & 0x01) == 0 ? 0 : 255;
				}
				subtotal += 1;
			}

		}
		++r;
		subtotal = 0;
	}
	return true;
}