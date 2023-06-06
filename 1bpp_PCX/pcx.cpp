#define esercizio1

#ifdef esercizio1
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

	int r = 0;
	int c = 0;
	for (size_t r = 0; r < img.rows(); ++r) {
		c = 0;
		while (c < totalbytes) {
			uint8_t b = 0;
			is.read(reinterpret_cast<char*>(&b), 1);
			if ((b >> 6) == 3) {
				uint8_t run = b & 0b00111111;
				uint8_t d;
				is.read(reinterpret_cast<char*>(&d), 1);

				while (run-- > 0) {
					for (uint8_t i = 0; i < 8; ++i) {
						img[r * img.cols() + (c * 8) + i] = ((d >> (8 - i - 1)) & 0x01) == 0 ? 0 : 255;
					}
					++c;
				}
			}
			else {
				size_t i = 8;
				for (uint8_t i = 0; i < 8; ++i) {
					img[r * img.cols() + (c * 8) + i] = ((b >> (8 - i - 1)) & 0x01) == 0 ? 0 : 255;
				}
				++c;
			}

		}
	}
	return true;
}

#endif // esercizio1

#ifdef esercizio2
#include "pcx.h"
#include "types.h"

#include <fstream>
#include <iostream>

bool read_header(std::ifstream& is, int& xsize, int& ysize, int& totalbytes, int& bppline) {
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
	bppline = int(Bppline);
	return true;
}


bool load_pcx(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "Cannot open input file." << std::endl;
		return false;
	}

	int xsize, ysize;
	int totalbytes;
	int bppline;

	if (!read_header(is, xsize, ysize, totalbytes, bppline)) {
		return false;
	}

	img.resize(ysize, xsize);

	for (size_t r = 0; r < img.rows(); ++r) {
		int plane = 0;
		int c = 0;
		
		while (plane < 3) {
			uint8_t b = 0;
			is.read(reinterpret_cast<char*>(&b), 1);
			if ((b >> 6) == 3) {
				uint8_t run = b & 0b00111111;
				uint8_t d;
				is.read(reinterpret_cast<char*>(&d), 1);

				while (run-- > 0) {
					img(r, c)[plane] = d;
					++c;
					if (c >= bppline) {
						c = 0;
						++plane;
					}
				}
			}
			else {
				img(r, c)[plane] = b;
				++c;
				if (c >= bppline) {
					c = 0;
					++plane;
				}
			}
		}
	}
	return true;
}

#endif // esercizio2

#ifdef esercizio3
#include "pcx.h"
#include "types.h"

#include <fstream>
#include <iostream>

bool read_header(std::ifstream& is, int& xsize, int& ysize, int& totalbytes, int& bppline) {
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
	bppline = int(Bppline);
	return true;
}

void read_palette(std::ifstream& is, std::vector<vec3b>& p) {
	const auto it = is.tellg();
	is.seekg(-769, std::ios::end);
	is.get();
	is.read(reinterpret_cast<char*>(p.data()), 256 * 3);
	is.seekg(it);
}

bool load_pcx(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "Cannot open input file." << std::endl;
		return false;
	}

	int xsize, ysize;
	int totalbytes;
	int bppline;

	if (!read_header(is, xsize, ysize, totalbytes, bppline)) {
		return false;
	}

	std::vector<vec3b> palette;
	palette.resize(256);
	read_palette(is, palette);

	img.resize(ysize, xsize);

	is.seekg(128);
	for (size_t r = 0; r < img.rows(); ++r) {
		int c = 0;

		while (c < totalbytes) {
			uint8_t b = 0;
			is.read(reinterpret_cast<char*>(&b), 1);
			if ((b >> 6) == 3) {
				uint8_t run = b & 0b00111111;
				uint8_t d;
				is.read(reinterpret_cast<char*>(&d), 1);

				while (run-- > 0) {
					img(r, c) = palette[d];
					++c;
				}
			}
			else {
				img(r, c) = palette[b];
				++c;
			}
		}
	}
	return true;
}

#endif // esercizio3
