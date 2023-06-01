#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <cstdint>

using vec3b = std::array<uint8_t, 3>;

template<typename T>
struct mat {
	uint32_t rows_;
	uint32_t cols_;
	std::vector<T> data_;

	mat(int r, int c) : rows_(r), cols_(c), data_(rows_*cols_) {}
	void resize(int r, int c) {
		rows_ = r;
		cols_ = c;
		data_.resize(rows_ * cols_);
	}
	
	T& operator() (int r, int c) {
		return data_[r * cols_ + c];
	}
	const T& operator() (int r, int c) const {
		return data_[r * cols_ + c];
	}

	int rows() const { return rows_; };
	int cols() const { return cols_; };
	int size() const { return cols_ * rows_; };
	char* rawdata() { return reinterpret_cast<char*>(&data_[0]); };
	const char* rawdata() const { return reinterpret_cast<const char*>(&data_[0]); };
	int rawsize() const { return cols_ * rows_ * sizeof(T); }
	auto begin() const { return data_.begin(); }
	auto end() const { return data_.end(); }
	auto begin() { return data_.begin(); }
	auto end() { return data_.end(); }
};

struct le2be {
	void operator()(uint16_t& n) {
		uint16_t res = 0;
		res += n >> 8;
		res += (n << 8) & 0b1111111100000000;
		n = res;
	}
};

void convert_16_to_8(mat<uint16_t>& im16, mat<uint8_t>& im8) {
	for (size_t i = 0; i < im16.rows(); ++i) {
		for (size_t j = 0; j < im16.cols(); ++j) {
			im8(i, j) = im16(i, j) / 256;
		}
	}
}

template<typename T>
void write_pgm(std::string ofile, mat<T> img) {
	std::ofstream os(ofile, std::ios::binary);
	if (!os) {
		std::cout << "Error: cannot open output file." << std::endl;
		exit(EXIT_FAILURE);
	}
	os << "P5 " << std::to_string(img.cols()) << " " << std::to_string(img.rows()) << " 255\n";
	os.write(reinterpret_cast<const char*>(img.rawdata()), img.rawsize());
}

template<typename T>
void write_ppm(std::string ofile, mat<T> img) {
	std::ofstream os(ofile, std::ios::binary);
	if (!os) {
		std::cout << "Error: cannot open output file." << std::endl;
		exit(EXIT_FAILURE);
	}
	os << "P6 " << std::to_string(img.cols()) << " " << std::to_string(img.rows()) << " 255\n";
	os.write(reinterpret_cast<const char*>(img.rawdata()), img.rawsize());
}

int sat(int x) {
	return (x > 255 ? 255 : x < 0 ? 0 : x);
}

void interpolate_firstPass(mat<vec3b>& img) {
	for (size_t r = 2; r < img.rows() - 2; ++r) {
		for (size_t c = 2; c < img.cols() - 2; c++) {
			uint8_t channel;
			if (r % 2 == 1) {//riga dispari
				if (c % 2 == 1) { //colonna dispari
					channel = 2;
				}
				else {
					channel = 1;
				}
			}
			else {//riga pari
				if (c % 2 == 1) { //colonna dispari
					channel = 1;
				}
				else {
					channel = 0;
				}
			}
			if (channel != 1){
				int g4 = img(r, c - 1)[1];
				int g6 = img(r, c + 1)[1];
				int g2 = img(r - 1, c)[1];
				int g8 = img(r + 1, c)[1];
				int x5 = img(r, c)[channel];
				int x3 = img(r, c - 2)[channel];
				int x7 = img(r, c + 2)[channel];
				int x1 = img(r - 2, c)[channel];
				int x9 = img(r + 2, c)[channel];
				int dH = std::abs(g4 - g6) + std::abs(x5 - x3 + x5 - x7);
				int dV = std::abs(g2 - g8) + std::abs(x5 - x1 + x5 - x9);
				int g5;
				
				if(dH < dV){
					g5 = (g4 + g6) / 2 + (x5 - x3 + x5 - x7) / 4;
					img(r, c)[1] = sat(g5);
				}
				else if (dH > dV) {
					g5 = (g2 + g8) / 2 + (x5 - x1 + x5 - x9) / 4;
					img(r, c)[1] = sat(g5);
				}
				else {
					g5 = (g2 + g4 + g6 + g8) / 4 + (x5 - x1 + x5 - x3 + x5 - x7 + x5 - x9) / 8;
					img(r, c)[1] = sat(g5);
				}
			}
		}
	}
}

void interpolate_secondPass(mat<vec3b>& img) {
	for (size_t r = 2; r < img.rows() - 2; ++r) {
		for (size_t c = 2; c < img.cols() - 2; c++) {
			uint8_t channel;
			bool red_topbottom;
			if (r % 2 == 1) {//riga dispari
				if (c % 2 == 1) { //colonna dispari
					channel = 2;
				}
				else {
					channel = 1;
					red_topbottom = true;
				}
			}
			else {//riga pari
				if (c % 2 == 1) { //colonna dispari
					channel = 1;
					red_topbottom = false;
				}
				else {
					channel = 0;
				}
			}
			if (channel == 1) {
				if (!red_topbottom) {
					img(r, c)[0] = sat((img(r, c - 1)[0] + img(r, c + 1)[0]) / 2);
					img(r, c)[2] = sat((img(r - 1, c)[2] + img(r + 1, c)[2]) / 2);
				}
				else {
					img(r, c)[0] = sat((img(r - 1, c)[0] + img(r + 1, c)[0]) / 2);
					img(r, c)[2] = sat((img(r, c - 1)[2] + img(r, c + 1)[2]) / 2);
				}
			}
			else {
				//if pixel 5 is R, B must be interpolated and vice versa.
				channel = channel == 0 ? 2 : 0;
				int x1 = img(r - 1, c - 1)[channel];
				int x3 = img(r - 1, c + 1)[channel];
				int x7 = img(r + 1, c - 1)[channel];
				int x9 = img(r + 1, c + 1)[channel];

				int g1 = img(r - 1, c - 1)[1];
				int g3 = img(r - 1, c + 1)[1];
				int g5 = img(r, c)[1];
				int g7 = img(r + 1, c - 1)[1];
				int g9 = img(r + 1, c + 1)[1];

				int dN = std::abs(x1 - x9) + std::abs(g5 - g1 + g5 - g9);
				int dP = std::abs(x3 - x7) + std::abs(g5 - g3 + g5 - g7);
				int x5;

				if (dN < dP) {
					x5 = (x1 + x9) / 2 + (g5 - g1 + g5 - g9) / 4;
					img(r, c)[channel] = sat(x5);
				}
				else if (dN > dP) {
					x5 = (x3 + x7) / 2 + (g5 - g3 + g5 - g7) / 4;
					img(r, c)[channel] = sat(x5);
				}
				else {
					x5 = (x1 + x3 + x7 + x9) / 4 + (g5 - g1 + g5 - g3 + g5 - g7 + g5 - g9) / 8;
					img(r, c)[channel] = sat(x5);
				}
			}
		}
	}
}

bool load_pgm(std::string ifile, std::string ofile) {
	std::ifstream is(ifile, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file." << std::endl;
		return false;
	}
	std::string mn;
	uint32_t w;
	uint32_t h;
	uint16_t mv;

	is >> mn >> w >> h >> mv;
	is.get();

	mat<uint16_t> pgm(h, w);
	is.read(reinterpret_cast<char*>(pgm.rawdata()), pgm.rawsize());

	le2be l2b;
	std::for_each(pgm.begin(), pgm.end(), l2b);
	mat<uint8_t> pgm_new(h, w);
	convert_16_to_8(pgm, pgm_new);

	write_pgm(ofile + "_gray.pgm", pgm_new);

	mat<vec3b> img(h, w);
	for (size_t i = 0; i < img.rows(); ++i) {
		for (size_t j = 0; j < img.cols(); j++) {
			if (i % 2 == 1) {//riga dispari
				if (j % 2 == 1){ //colonna dispari
					img(i, j)[2] = pgm_new(i, j); //scrivi B
				}
				else {
					img(i, j)[1] = pgm_new(i, j); //scrivi G
				}
			}
			else {//riga pari
				if (j % 2 == 1) { //colonna dispari
					img(i, j)[1] = pgm_new(i, j); //scrivi G
				}
				else {
					img(i, j)[0] = pgm_new(i, j); //scrivi R
				}
			}
		}
	}

	write_ppm(ofile + "_bayer.ppm", img);

	interpolate_firstPass(img);

	write_ppm(ofile + "_green.ppm", img);

	interpolate_secondPass(img);
	
	write_ppm(ofile + "_interp.ppm", img);
	return true;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Syntax error." << std::endl;
		return(EXIT_FAILURE);
	}

	std::string ifile = argv[1];
	std::string ofile = argv[2];

	load_pgm(ifile, ofile);
	
	return(EXIT_SUCCESS);
}