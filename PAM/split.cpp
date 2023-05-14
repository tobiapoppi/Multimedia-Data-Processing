#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <array>

//these are commented because OLJ doens't have them and I must upload a single file.
// 
//#include "types.h"
//#include "mat.h"


using vec3b = std::array<uint8_t, 3>;
template<typename T>
class mat {
	int rows_ = 0, cols_ = 0;
	std::vector<T> data_;
public:
	mat() {}
	mat(int rows, int cols) : rows_(rows), cols_(cols), data_(rows* cols) {}
	int rows() const { return rows_; }
	int cols() const { return cols_; }
	int size() const { return data_.size(); }

	T& operator() (int r, int c) {
		return data_[r * cols_ + c];
	}
	const T& operator() (int r, int c) const {
		return data_[r * cols_ + c];
	}
	char* rawdata() { return reinterpret_cast<char*>(&data_[0]); }
	const char* rawdata() const { return reinterpret_cast<const char*>(&data_[0]); }
	int rawsize() const { return rows_ * cols_ * sizeof(T); }
	auto begin() { return data_.begin(); }
	auto begin() const { return data_.begin(); }
	auto end() { return data_.end(); }
	auto end() const { return data_.end(); }
};


void write_pam(std::string out_filename, mat<vec3b>& img, uint32_t h, uint32_t w, uint8_t channel) {
	std::ofstream os(out_filename, std::ios::binary);
	os << "P7" << '\n' << "WIDTH " << std::to_string(w) << '\n' << "HEIGHT " << std::to_string(h) << '\n' << "DEPTH " << "1" << '\n';
	os << "MAXVAL 255\nTUPLTYPE GRAYSCALE\nENDHDR\n";
	for (size_t i = 0; i < img.rows(); ++i) {
		for (size_t j = 0; j < img.cols(); ++j) {
			os.write(reinterpret_cast<const char*>(&img(i, j)[channel]), 1);
		}
	}
}

/*
int main(int argc, char** argv) {
	std::string filename = argv[1];
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file." << std::endl;
		return EXIT_FAILURE;
	}
	std::string s;
	uint32_t h = 0;
	uint32_t w = 0;
	uint8_t d = 0;
	uint32_t mv = 0;
	std::string tupletype;

	is >> s;
	char consume;
	consume = is.get();
	while (s != "ENDHDR") {
		is >> s;
		if (s.front() == '#') {
			std::getline(is, s);
		}
		if (s == "HEIGHT") {
			is >> h;
		}
		else if (s == "WIDTH") {
			is >> w;
		}
		else if (s == "DEPTH") {
			is >> d;
		}
		else if (s == "MAXVAL") {
			is >> mv;
		}
		else if (s == "TUPLTYPE") {
			is >> s;
			tupletype += s;
		}
	}
	is.get();

	//read image data
	mat<vec3b> img(h, w);
	is.read(reinterpret_cast<char*>(img.rawdata()), img.rawsize());

	//write three channel images
	std::string fr;
	std::string fg;
	std::string fb;
	fr = filename.substr(0, filename.find_last_of("."));
	fg = fr + "_G.pam";
	fb = fr + "_B.pam";
	fr += "_R.pam";

	write_pam(fr, img, h, w, 0);
	write_pam(fg, img, h, w, 1);
	write_pam(fb, img, h, w, 2);

}*/