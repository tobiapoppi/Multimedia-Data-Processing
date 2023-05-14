#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <array>


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
	char* rawdata() { return reinterpret_cast<char*>(&data_[0]); }
	const char* rawdata() const { return reinterpret_cast<const char*>(&data_[0]); }
	int rawsize() const { return rows_ * cols_ * sizeof(T); }
	auto begin() { return data_.begin(); }
	auto begin() const { return data_.begin(); }
	auto end() { return data_.end(); }
	auto end() const { return data_.end(); }
};


void read_pam(std::string filename, mat<uint8_t>& img, uint8_t channel) {
	std::string fn = filename;
	fn = filename.substr(0, filename.find_last_of("."));

	switch (channel) {
	case 0:
		fn += "_R.pam";
		break;
	case 1:
		fn += "_G.pam";
		break;
	case 2:
		fn += "_B.pam";
		break;
	}
	std::ifstream is(fn, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file." << std::endl;
		exit(EXIT_FAILURE);
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
	img.resize(h, w);
	is.read(reinterpret_cast<char*>(img.rawdata()), img.rawsize());
}

void comb(mat<uint8_t>& r, mat<uint8_t>& g, mat<uint8_t>& b, mat<vec3b>& im){
	im.resize(r.rows(), r.cols());
	for (size_t i = 0; i < im.rows(); ++i) {
		for (size_t j = 0; j < im.cols(); ++j) {
			im(i, j)[0] = r(i, j);
			im(i, j)[1] = g(i, j);
			im(i, j)[2] = b(i, j);
		}
	}
}

void write_pam(std::string out_filename, mat<vec3b>& img) {
	std::ofstream os(out_filename, std::ios::binary);
	os << "P7" << '\n' << "WIDTH " << std::to_string(img.cols()) << '\n' << "HEIGHT " << std::to_string(img.rows()) << '\n' << "DEPTH " << "3" << '\n';
	os << "MAXVAL 255\nTUPLTYPE RGB\nENDHDR\n";
	for (size_t i = 0; i < img.rows(); ++i) {
		for (size_t j = 0; j < img.cols(); ++j) {
			os.write(reinterpret_cast<const char*>(&img(i, j)), 3);
		}
	}
}
/*
int main(int argc, char** argv) {
	std::string filename = argv[1];
	std::string out_f;
	out_f = filename.substr(0, filename.find_last_of("."));
	out_f += "_reconstructed.pam";

	mat<uint8_t> red;
	mat<uint8_t> green;
	mat<uint8_t> blue;
	mat<vec3b> img;

	read_pam(filename, red, 0);
	read_pam(filename, green, 1);
	read_pam(filename, blue, 2);
	comb(red, green, blue, img);
	write_pam(out_f, img);
}*/