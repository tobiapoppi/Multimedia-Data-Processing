#include <fstream>
#include <string>
#include <iostream>
#include <array>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <cmath>

using rgbe = std::array<uint8_t, 4>;
using rgbf = std::array<float, 3>;
using rgb = std::array<uint8_t, 3>;

template<typename T>
struct mat {
	int r_;
	int c_;
	std::vector<T> data_;

	mat(int r, int c) : r_(r), c_(c), data_(r* c) {}
	void resize(int r, int c) {
		r_ = r;
		c_ = c;
		data_.resize(r * c);
	}

	int rows() { return r_; }
	const int rows() const { return r_; }
	int cols() { return c_; }
	const int cols() const { return c_; }

	T* rawdata() { return data_.data(); }
	const T* rawdata() const { return &data_.data(); }
	int rawsize() { return r_ * c_ * sizeof(T); }
	const int rawsize() const { return r_ * c_ * sizeof(T); }
	T& operator() (int r, int c) {
		return data_[(r * c_) + c];
	}
};

void rgbe_2_rgbf(mat<rgbe>& im4, mat<rgbf>& im3) {
	for (size_t r = 0; r < im4.rows(); ++r) {
		for (size_t c = 0; c < im4.cols(); ++c) {
			float Rf, Gf, Bf;
			float R = float(im4(r, c)[0]);
			float G = float(im4(r, c)[1]);
			float B = float(im4(r, c)[2]);
			Rf = ((R + 0.5) / float(256)) * std::pow(2, float(im4(r, c)[3] - 128));
			Gf = ((G + 0.5) / float(256)) * std::pow(2, float(im4(r, c)[3] - 128));
			Bf = ((B + 0.5) / float(256)) * std::pow(2, float(im4(r, c)[3] - 128));
			im3(r, c)[0] = Rf;
			im3(r, c)[1] = Gf;
			im3(r, c)[2] = Bf;
		}
	}
}

void rgbf_2_rgb(mat<rgbf>& im3, mat<rgb>& im) {

	//find min and max
	float max = -1;
	float min = 1;
	for (size_t r = 0; r < im3.rows(); ++r) {
		for (size_t c = 0; c < im3.cols(); ++c) {
			for (size_t p = 0; p < 3; ++p) {
				if (im3(r, c)[p] > max) max = float(im3(r, c)[p]);
				if (im3(r, c)[p] < min) min = float(im3(r, c)[p]);
			}
		}
	}

	//calculate integers from floats
	for (size_t r = 0; r < im3.rows(); ++r) {
		for (size_t c = 0; c < im3.cols(); ++c) {
			for (size_t p = 0; p < 3; ++p) {
				float f = im3(r, c)[p];
				float val = float(255) * (std::pow((f - min) / (max - min), 0.45));
				im(r, c)[p] = uint8_t(val);
			}
		}
	}

}

bool write_pam(std::string ofile, mat<rgb>& im) {
	std::ofstream os(ofile, std::ios::binary);
	if (!os) {
		return false;
	}
	os << "P7\nWIDTH " << std::to_string(im.cols()) << "\nHEIGHT " << std::to_string(im.rows()) << "\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n";
	os.write(reinterpret_cast<const char*>(im.rawdata()), im.rawsize());
	return true;
}

bool load_hdr(std::string ifile, mat<rgb>& rgb_image) {
	std::ifstream is(ifile, std::ios::binary);
	if (!is) {
		return false;
	}
	std::string mn;
	is.read(reinterpret_cast<char*>(&mn[0]), 10);

	std::string format;

	is.get(); //get newline

	while (is.peek() != '\n') {
		if (is.peek() != '#') {
			std::string name;
			std::string val;
			while (is.peek() != '=') {
				name += is.get();
			}
			is.get();
			is >> val;
			if (name == "FORMAT") {
				format = val;
			}
		}
		else {
			std::string thrash;
			std::getline(is, thrash);
		}
	}

	std::string thrash;
	std::string nstring, mstring;
	is >> thrash >> nstring >> thrash >> mstring;
	is.get(); //get newline

	int n = std::stoi(nstring);
	int m = std::stoi(mstring);

	mat<rgbe> im(n, m);

	for (size_t r = 0; r < n; ++r) {
		int plane = 0;
		int count = 0;
		is.ignore(4);

		while (plane < 4) {
			while (count < m) {
				uint8_t L;
				is.read(reinterpret_cast<char*>(&L), 1);

				if (L <= 127) {
					//copy L
					for (size_t i = 0; i < L; ++i) {
						uint8_t val;
						is.read(reinterpret_cast<char*>(&val), 1);
						auto it = is.tellg();
						im(r, count)[plane] = val;
						++count;
					}
				}
				else {
					//run L - 128
					uint8_t val;
					is.read(reinterpret_cast<char*>(&val), 1);
					for (size_t i = 0; i < (L - 128); ++i) {
						im(r, count)[plane] = val;
						++count;
					}
				}
			}
			++plane;
			count = 0;
		}
	}

	mat<rgbf> im_rgbf(n, m);
	rgbe_2_rgbf(im, im_rgbf);

	rgb_image.resize(n, m);
	rgbf_2_rgb(im_rgbf, rgb_image);

	return true;

}

int main(int argc, char** argv) {
	if (argc != 3) {
		return EXIT_FAILURE;
	}

	std::string ifile = argv[1];
	std::string ofile = argv[2];

	mat<rgb> im(0, 0);
	
	if (!load_hdr(ifile, im)) return EXIT_FAILURE;
	if (!write_pam(ofile, im)) return EXIT_FAILURE;
	
	return EXIT_SUCCESS;
}