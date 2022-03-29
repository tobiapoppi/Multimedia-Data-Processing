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
		<< "frequencies <input_file.txt> <output_file.txt>\n\n"
		<< "Writes numbers as 32bit little endian.";
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "ERROR: " << msg << "\n";
	exit(EXIT_FAILURE);
}

class bitwriter {
	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::ostream& os_;

	//streams cannot be copied. But i can take a reference to an external out stream.
	//here is mandatory o have the initialization in the constructor.

	void write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | bit;
		++nbits_;
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

public:

	bitwriter(std::ostream& os) : os_(os) {}
	~bitwriter() {
		flush();
	}

	std::ostream& write(uint32_t u, uint8_t n) {
		while (n-- > 0) {  //it's like n-- > 0 but easy to remember and we have control on n, not n-1.
			uint8_t bit = (u >> n) & 0b00000001;
			write_bit(bit);
		}
		return os_;
	}

	void flush(uint8_t bit = 0) {
		while (nbits_ > 0) {
			write_bit(bit);
		}
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

	std::vector<int32_t> v{
		std::istream_iterator<int32_t>(is),
		std::istream_iterator<int32_t>()
	};

	std::ofstream os(output_filename, std::ios::binary);
	if (!os) {
		error("Cannot open file " + output_filename);
	}

	uint8_t buffer;
	uint8_t nbits = 0;

	bitwriter bw(os);

	for (const auto& x : v) {
		bw.write(x, 11);
	}

	return EXIT_SUCCESS;
}

