#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <string>
#include <array>
#include <cstdint>
#include <iomanip>

void syntax() {
	std::cout << "SYNTAX:\n\n"
		<< "read_int32 <input_file> <output_file.txt>\n\n"
		<< "Reads numbers as 32bit little endian.";
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "ERROR: " << msg << "\n";
	exit(EXIT_FAILURE);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

struct bitreader {
	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::istream& is_;

	bitreader(std::istream& is) : is_(is) {}

	int read_bit() {
		if (nbits_ == 0) {
			if (!raw_read(is_, buffer_)) {
				return EOF;
			}
			nbits_ = 0;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

	explicit operator bool(){ return bool(is_); }
	bool operator!() { return !is_; }

	std::istream& read(uint32_t& u, uint8_t n) {
		u = 0;
		while (n --> 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}

};

int main(int argc, char* argv[]) {
	if (argc != 3) {
		syntax();
	}
	std::string input_filename = argv[1];
	std::string output_filename = argv[2];

	std::ifstream is(input_filename);
	if (!is) {
		error("Cannot open file " + input_filename);
	}

	std::vector<int32_t> v;
	bitreader br(is);
	uint32_t u;
	while (br.read(u, 11)) {
		v.push_back(u);
	}

	std::ofstream os(output_filename, std::ios::out);
	if (!os) {
		error("Cannot open file " + output_filename);
	}

	copy(v.begin(), v.end(), std::ostream_iterator<int32_t>(os, "\n"));

	return EXIT_SUCCESS;
}

