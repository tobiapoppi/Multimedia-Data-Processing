#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <array>

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

	T& operator(int r, int c) { return data_[(r * c_) + c]; }
	const T& operator(int r, int c) const { return data_[(r * c_) + c]; }

	auto rawdata() { return data_.data(); }
	int rawsize() { return r_ * c_ * sizeof(T); }

};

bool read_pam_header(std::ifstream& is, int& width, int& height, bool& a) {
	std::string s;
	
	int w, h, d, mv;

	while (s != "ENDHDR") {
		is >> s;
		if (s == "WIDTH") {
			std::string tmp;
			is >> tmp;
			w = std::stoi(tmp);
			width = w;
		}
		else if (s == "HEIGHT") {
			std::string tmp;
			is >> tmp;
			h = std::stoi(tmp);
			height = h;
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
	if (d == 4) {
		a = true;
	}
	else if (d == 3) {
		a = false;
	}
	else {
		std::cout << "Error: input file cannot be grayscale." << std::endl;
		return false;
	}
	return true;
}

template<typename T>
mat<T>& read_pam_data(std::ifstream& is, mat<T>& im) {
	if (a) {
		//RGB
		mat<rgb> im(h, w);
		is.read(reinterpret_cast<char*>(im.rawdata()), im.rawsize());
		return im;
	}
	else if (d == 4) {
		//RGBA
		mat<rgba> im(h, w);
		is.read(reinterpret_cast<char*>(im.rawdata()), im.rawsize());
		return im;

	}

}

bool compose(std::string ifile1, std::string ifile2, std::string ofile, int x_off_1, int y_off_1, int x_off_2, int y_off_2) {
	std::ifstream is(ifile1, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file." << std::endl;
		return false;
	}
	int w, h;
	bool a;
	read_pam_header(is, w, h, a);
	if (a) {
		mat<rgba> im(h, w);
		auto im = read_pam_data<rgba>(is, im);
	}

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
	if (compose(ifile1, ifile2, ofile, x_off_1, y_off_1, x_off_2, y_off_2);) {
		return EXIT_SUCCESS;
	}
}