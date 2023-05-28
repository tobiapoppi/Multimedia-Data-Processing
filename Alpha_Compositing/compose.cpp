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
	mat() : r_(0), c_(0), data_(0) {}
	mat(int r, int c) : r_(r), c_(c), data_(r* c*) {}

	void resize(int r, int c) {
		r_ = r;
		c_ = c;
		data_.resize(r, c);
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
	const T& operator(int r, int c) const { return data_[(r * c_) + c]; }

	auto get_pixel_addr(int r, int c) { return &data_[(r * c_) + c]; }

	auto rawdata() { return data_.data(); }
	int rawsize() { return r_ * c_ * sizeof(T); }

};

mat<rgba>& read_pam(std::string ifile) {
	std::string s;
	std::ifstream is(ifile, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file." << std::endl;
		mat<rgba> im(0, 0);
		return im;
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
		mat<rgba> im(h, w);
		for (size_t r; r < h; ++r) {
			for (size_t c; c < w; ++c) {
				is.read(reinterpret_cast<char*>(&im.at(r, c)), 3);
				im.at(r, c)[3] = 255;
			}
		}
		return im;
	}
	else if (d == 4){
		//RGBA
		mat<rgba> im(h, w);
		is.read(reinterpret_cast<char*>(im.rawdata()), im.rawsize());
		return im;

	}
	else {
		mat<rgba> im(0, 0);
		std::cout << "Error: input file cannot be grayscale." << std::endl;
		return im;
	}

}

void paste_img(mat<rgba>& new_im, mat<rgba>& im, int x_off, int y_off) {
	for (int r = 0; r < im.rows(); ++r) {
		for (int c = 0; c < im.cols(); ++c) {
			new_im.at(r + y_off, c + x_off) = im.at(r, c);
		}
	}
}

bool compose(std::string ifile1, std::string ifile2, std::string ofile, int x_off_1, int y_off_1, int x_off_2, int y_off_2) {
	
	auto im1 = read_pam(ifile1);
	auto im2 = read_pam(ifile2);

	int new_w = std::max(im1.cols(), im2.cols()) + std::max(x_off_1, x_off_2);
	int new_h = std::max(im1.rows(), im2.rows()) + std::max(y_off_1, y_off_2);

	mat<rgba> new_im(new_h, new_w);
	
	//paste first image and second image in new one
	paste_img(new_im, im1, x_off_1, y_off_1);
	paste_img(new_im, im2, x_off_2, y_off_2);

	//TODO: calculate alpha

	return true;
}

int main(int argc, char** argv) {
	std::string ofile = argv[1];
	ofile += ".pam";
	std::string ifile1;
	std::string ifile2;

	int x_off_1 = 0;
	int y_off_1 = 0;
	int x_off_2 = 0;
	int y_off_2 = 0;

	if (argv[2] == "-p") {
		x_off_1 = std::stoi(argv[3]);
		y_off_1 = std::stoi(argv[4]);
		ifile1 = argv[5];
		ifile1 += ".pam";


		if (argv[6] == "-p") {
			x_off_2 = std::stoi(argv[7]);
			y_off_2 = std::stoi(argv[8]);
			ifile2 = argv[9];
			ifile2 += ".pam";

		}
		else {
			ifile2 = argv[6];
			ifile2 += ".pam";

		}
	}
	else {
		ifile1 = argv[2];
		ifile1 += ".pam";


		if (argv[3] == "-p") {
			x_off_2 = std::stoi(argv[4]);
			y_off_2 = std::stoi(argv[5]);
			ifile2 = argv[6];
			ifile2 += ".pam";

		}
		else {
			ifile2 = argv[3];
			ifile2 += ".pam";
		}
	}
	if (compose(ifile1, ifile2, ofile, x_off_1, y_off_1, x_off_2, y_off_2)) {
		return EXIT_SUCCESS;
	}
}