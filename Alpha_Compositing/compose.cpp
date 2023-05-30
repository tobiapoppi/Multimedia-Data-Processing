#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <array>
#include <variant>

using rgb = std::array<uint8_t, 3>;
using rgba = std::array<uint8_t, 4>;

template<typename T>
class mat {
	uint32_t r_;
	uint32_t c_;
	std::vector<T> data_;

public:
	mat(int r, int c) : r_(r), c_(c), data_(r * c) {}

	void resize(int r, int c) {
		r_ = r;
		c_ = c;
		data_.resize(r*c);
	}

	void clear() {
		data_.clear();
		r_ = 0;
		c_ = 0;
	}

	int rows() { return r_; }
	int rows() const { return r_; }
	int cols() { return c_; }
	int cols() const { return c_; }
	int size() { return data_.size(); }
	int size() const { return data_.size(); }

	auto& begin() { return data_.begin(); }
	const auto& begin() const { return data_.begin(); }
	auto& end() { return data_.end(); }
	const auto& end() const { return data_.end(); }

	T& at(int r, int c) { return data_[(r * c_) + c]; }
	const T& operator()(int r, int c) const { return data_[(r * c_) + c]; }

	auto get_pixel_addr(int r, int c) { return &data_[(r * c_) + c]; }

	auto rawdata() { return data_.data(); }
	int rawsize() { return r_ * c_ * sizeof(T); }

};

bool read_pam(std::string ifile, mat<rgba>& im) {
	std::string s;
	std::ifstream is(ifile, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file." << std::endl;
		return false;
	}
	int w, h, d, mv;

	while (s != "ENDHDR") {
		is >> s;
		if (s == "WIDTH") {
			std::string tmp;
			is >> tmp;
			w = std::stoi(tmp);
		}
		else if (s == "HEIGHT") {
			std::string tmp;
			is >> tmp;
			h = std::stoi(tmp);
		}
		else if (s == "DEPTH") {
			std::string tmp;
			is >> tmp;
			d = std::stoi(tmp);
		}
		else if (s == "MAXVAL") {
			std::string tmp;
			is >> tmp;
			mv = std::stoi(tmp);
		}
		else if (s == "TUPLTYPE") {
			std::string tmp;
			is >> tmp;
		}
	}
	is.get();
	if (d == 3) {
		//RGB
		im.resize(h, w);
		for (size_t r = 0; r < h; ++r) {
			for (size_t c = 0; c < w; ++c) {
				is.read(reinterpret_cast<char*>(&im.at(r, c)), 3);
				im.at(r, c)[3] = 255;
			}
		}
		return true;
	}
	else if (d == 4){
		//RGBA
		im.resize(h, w);
		is.read(reinterpret_cast<char*>(im.rawdata()), im.rawsize());
		return true;

	}
	else {
		std::cout << "Error: input file cannot be grayscale." << std::endl;
		return false;
	}

}

bool write_pam(std::ofstream& os, mat<rgba>& im) {
	os << "P7\nWIDTH " << im.cols() << "\nHEIGHT " << im.rows() << "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
	os.write(reinterpret_cast<const char*>(im.rawdata()), im.rawsize());
	return true;
}

void paste_img(mat<rgba>& new_im, mat<rgba>& im, int x_off, int y_off) {
	for (int r = 0; r < im.rows(); ++r) {
		for (int c = 0; c < im.cols(); ++c) {
			new_im.at(r + y_off, c + x_off) = im.at(r, c);
		}
	}
}

bool compose(mat<rgba>& out, std::string ifile, int x_off, int y_off) {
	
	mat<rgba> im_to_add(0, 0);
	if (!read_pam(ifile, im_to_add)) {
		std::cout << "Cannot read input PAM image." << std::endl;
		return false;
	}

	int new_w = im_to_add.cols() > out.cols() ? im_to_add.cols() + x_off : im_to_add.cols() + x_off > out.cols() ? im_to_add.cols() + x_off : out.cols();
	int new_h = im_to_add.rows() > out.rows() ? im_to_add.rows() + y_off : im_to_add.rows() + y_off > out.rows() ? im_to_add.rows() + y_off : out.rows();

	mat<rgba> partial_im_1(new_h, new_w);
	mat<rgba> partial_im_2(new_h, new_w);
	
	//paste first image and second image in new one
	paste_img(partial_im_1, out, 0, 0);
	paste_img(partial_im_2, im_to_add, x_off, y_off);

	
	out.clear();
	out.resize(new_h, new_w);

	//calculate alpha
	for (int r = 0; r < out.rows(); ++r) {
		for (int c = 0; c < out.cols(); ++c) {
			out.at(r, c)[3] = partial_im_1.at(r, c)[3] + (partial_im_2.at(r, c)[3] * (1 - partial_im_1.at(r, c)[3]));
			
			if (out.at(r, c)[3] != 0) {
				for (uint8_t i = 0; i < 3; ++i) {
					out.at(r, c)[i] = ((partial_im_1.at(r, c)[i] * partial_im_1.at(r, c)[3]) + ((partial_im_2.at(r, c)[i] * partial_im_2.at(r, c)[3]) * (1 - partial_im_1.at(r, c)[3]))) / out.at(r, c)[3];
				}
			}
			else {
				for (uint8_t i = 0; i < 3; ++i) {
					out.at(r, c)[i] = partial_im_2.at(r, c)[i];
				}
			}
		}
	}
	return true;
}

int main(int argc, char** argv) {
	std::string ofile = argv[1];
	ofile += ".pam";
	std::string ifile;
	int nparams = argc - 2;
	int p = 2;
	mat<rgba> out(0, 0);

	while (nparams > 0) {
		int x_off = 0;
		int y_off = 0;
		if (std::string(argv[p]) == "-p") {
			x_off = std::stoi(argv[p + 1]);
			y_off = std::stoi(argv[p + 2]);
			p += 3;
			nparams -= 3;
		}
		ifile = argv[p];
		ifile += ".pam";
		if (!compose(out, ifile, x_off, y_off)) {
			return EXIT_FAILURE;
		}
		++p;
		nparams--;
	}

	std::ofstream os(ofile, std::ios::binary);
	if (!write_pam(os, out)) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}