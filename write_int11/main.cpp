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

	for (const auto& x : v) {
		for (int n = 10; n > 0; --n) {
			//for each of the lower 11 bits of x
			uint8_t bitextracted = (x >> n) & 0b00000001; //i can also write in hex with 0x01
														  //or & 1 in decimal version.
														  // !!! && is not bitwise !!
			//puth it in a buffer from the lower end. 
			buffer = (buffer << 1) | bitextracted;
			++nbits;
			//If the buffer is full (8 bits) write it to file.
			if (nbits == 8) {
				os.write(reinterpret_cast<char*>(&buffer), 1);
				nbits = 0;
			}
		}
	}

	while (0 < nbits && nbits < 8) {
		uint8_t bit = 0;
		buffer = (buffer << 1) | bit;
		++nbits;
		if (nbits == 8) {
			os.write(reinterpret_cast<const char*>(&buffer), 1);
		}
	}

	return EXIT_SUCCESS;
}

