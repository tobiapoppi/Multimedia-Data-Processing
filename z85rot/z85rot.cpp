#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

static uint64_t count = 0;


void error(const char* msg) {
	std::cout << msg;
	exit(EXIT_FAILURE);
}

template<typename T>
class mat {
	uint32_t r_;
	uint32_t c_;
	std::vector<T> data_;
	uint8_t bytesfornum_ = 1;
	uint8_t p_ = 0;

public:
	mat(int r = 0, int c = 0) {
		r_ = r;
		c_ = c;
	}

	auto rawdata() { return &data_[0]; }
	auto begin() { return data_.begin(); }
	const auto begin() const { return data_.begin(); }
	auto end() { return data_.end(); }
	const auto end() const { return data_.end(); }
	auto rows() const { return r_; }
	auto cols() const { return c_; }
	auto size() const { return (c_ * r_ * 3 * bytesfornum_) + p_; }
	auto bytesfornum() const { return bytesfornum_; };


	void change_bytes_for_num(int b) {
		bytesfornum_ = b;
	}

	void resize(int r, int c, int p = 0) { 
		r_ = r;
		c_ = c;
		p_ = p;
		data_.resize((r_ * c_ * 3 * bytesfornum_ ) + p_);
	}

	T& operator[](int i) {
		return data_[i];
	}
	const T& operator[] (int i) const {
		return data_[i];
	}
};

mat<uint8_t> read_ppm(char* filename) {
	mat<uint8_t> m(0, 0);
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return m;
	}
	std::string mn;
	char trash;
	is >> mn;
	if (mn != "P6") {
		return m;
	}
	is.get(trash); //get newline
	if (is.peek() == '#') {
		std::string comment;
		std::getline(is, comment);
	}
	std::string s;
	uint32_t w;
	uint32_t h;
	uint32_t maxval;
	is >> s;
	w = std::stoi(s);
	is >> s;
	h = std::stoi(s); 
	is >> s;
	maxval = std::stoi(s);
	if (maxval == 0 || maxval >= 65536) {
		return m;
	}
	int padding = 0;
	if (maxval >= 256) {
		m.change_bytes_for_num(2);
	}
	//consume space/newline
	{
		char ws;
		is.get(ws);
	}
	m.resize(h, w);
	if (m.size() % 4 != 0) {
		padding = 4 - (m.size() % 4);
	}
	m.resize(h, w, padding);

	//read data
	for (size_t i = 0; i < m.size(); ++i) {
		uint8_t tmp;
		if (is.read(reinterpret_cast<char*>(&tmp), 1)) {
			m[i] = tmp;
		}
		else {
			m[i] = 0;
		}
	}
	return m;
}



char z85_table(int8_t n, int N) {
	n = n - (N * count);
	
	while (n < 0) {
		n = n + 85;
	}
	while (n > 84) {
		n = n - 85;
	}
	++count;

	if (n >= 0 && n <= 9) { return n + 48; }
	if (n >= 10 && n <= 35) { return n + 87; }
	if (n >= 36 && n <= 61) { return n + 29; }
	if (n == 62) { return '.'; }
	if (n == 63) { return '-'; }
	if (n == 64) { return ':'; }
	if (n == 65) { return '+'; }
	if (n == 66) { return '='; }
	if (n == 67) { return '^'; }
	if (n == 68) { return '!'; }
	if (n == 69) { return '/'; }
	if (n == 70) { return '*'; }
	if (n == 71) { return '?'; }
	if (n == 72) { return '&'; }
	if (n == 73) { return '<'; }
	if (n == 74) { return '>'; }
	if (n == 75) { return '('; }
	if (n == 76) { return ')'; }
	if (n == 77) { return '['; }
	if (n == 78) { return ']'; }
	if (n == 79) { return '{'; }
	if (n == 80) { return '}'; }
	if (n == 81) { return '@'; }
	if (n == 82) { return '%'; }
	if (n == 83) { return '$'; }
	if (n == 84) { return '#'; }
}

char z85decode(int8_t c, int N) {
	int8_t v = 0;
	if (c >= '0' && c <= '9') { v = c - 48; }
	if (c >= 'a' && c <= 'z') { v = c - 87; }
	if (c >= 'A' && c <= 'Z') { v = c - 29; }
	if (c == '.') { v = 62; }
	if (c == '-') { v = 63; }
	if (c == ':') { v = 64; }
	if (c == '+') { v = 65; }
	if (c == '=') { v = 66; }
	if (c == '^') { v = 67; }
	if (c == '!') { v = 68; }
	if (c == '/') { v = 69; }
	if (c == '*') { v = 70; }
	if (c == '?') { v = 71; }
	if (c == '&') { v = 72; }
	if (c == '<') { v = 73; }
	if (c == '>') { v = 74; }
	if (c == '(') { v = 75; }
	if (c == ')') { v = 76; }
	if (c == '[') { v = 77; }
	if (c == ']') { v = 78; }
	if (c == '{') { v = 79; }
	if (c == '}') { v = 80; }
	if (c == '@') { v = 81; }
	if (c == '%') { v = 82; }
	if (c == '$') { v = 83; }
	if (c == '#') { v = 84; }
	v = v + (N * count);
	while (v < 0) {
		v = v + 85;
	}
	while (v > 84) {
		v = v - 85;
	}
	++count;

	return static_cast<uint8_t>(v);
}

std::vector<uint8_t> b10tob85(uint32_t val) {
	std::vector<uint8_t> r;
	while (val != 0) {
		r.push_back(val % 85);
		val = val / 85;
	}
	while (r.size() != 5) {
		r.push_back(0);
	}
	std::reverse(r.begin(), r.end());
	return r;
}

uint32_t b85tob10(std::vector<uint8_t>& v) {
	std::reverse(v.begin(), v.end());
	uint32_t val = 0;
	for (size_t i = 0; i < 5; ++i) {
		val += v[i] * std::pow(85,i);
	}
	return val;
}

template<typename T>
void write_txt(char* filename, const mat<T>& m, int N) {
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		error("Cannot open output file.");
	}
	os << m.cols() << ',' << m.rows() << ',';
	for (size_t i = 0; i < m.size() / 4; ++i) {
		uint32_t val = 0;
		val += (m[i * 4 + 0] << 24);
		val += (m[i * 4 + 1] << 16);
		val += (m[i * 4 + 2] << 8);
		val += m[i * 4 + 3];
		auto b85 = b10tob85(val);
		for (size_t k = 0; k < 5; ++k) {
			os << z85_table(b85[k], N);
		}
	}
}

void read_txt(char* in_filename, char* out_filename, int N) {
	std::ifstream is(in_filename, std::ios::binary);
	if (!is) {
		error("Cannot open input file.");
	}
	std::ofstream os(out_filename, std::ios::binary);
	if (!os) {
		error("Cannot open output file.");
	}
	std::string s;
	std::string c;
	std::string r;
	char trash;
	std::getline(is, c, ',');
	std::getline(is, r, ',');

	uint8_t val;
	uint32_t dec;
	std::vector<uint8_t> data;
	uint32_t maxval = 0;

	os << "P6\n" << c << ' ' << r << '\n';
	while (is.peek() != EOF) {
		std::vector<uint8_t> v;
		for (size_t i = 0; i < 5; ++i) {
			is.read(reinterpret_cast<char*>(&val), 1);
			val = z85decode(val, N);
			v.push_back(val);
		}
		dec = b85tob10(v);
		uint8_t a;
		a = dec >> 24;
		data.push_back(a);
		if (a > maxval) {
			maxval = a;
		}
		a = dec >> 16;
		data.push_back(a);
		if (a > maxval) {
			maxval = a;
		}
		a = dec >> 8;
		data.push_back(a);
		if (a > maxval) {
			maxval = a;
		}
		a = dec;
		data.push_back(a);
		if (a > maxval) {
			maxval = a;
		}
	}
	os << +maxval << '\n';
	int size = std::stoi(c) * std::stoi(r) * 3;
	std::cout << size <<"\n";
	std::cout << data.size() << "\n";
	for (size_t i = 0; i < size; ++i) {
		os.write(reinterpret_cast<char*>(&data[i]), 1);
	}
}

int main(int argc, char* argv[]) {

	if (argc != 5) {
		error("SYNTAX: Z85rot {c | d} <N> <input file> <output file>");
	}

	if (argv[1][0] == 'c') {
		//read input file
		auto img = read_ppm(argv[3]);
		int n = std::stoi(argv[2]);
		write_txt(argv[4], img, n);
	}

	else if (argv[1][0] == 'd') {
		//read input file
		int n = std::stoi(argv[2]);
		read_txt(argv[3], argv[4], n);
	}
	else {
		error("Syntax: mode must be 'c' or 'd'.");
	}

	return EXIT_SUCCESS;
}