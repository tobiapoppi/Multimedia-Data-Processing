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
std::istream& raw_read(const std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
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

	std::vector<int32_t> v; //we can't use iterators this time
	int32_t val;

	//I assume Little Endian Architecture
	/*raw_read(is, val);
	is.read(reinterpret_cast<char*>(&val), sizeof(val));
	//Read Little Endian independent of the architecture
	uint8_t vals[4];
	is.read(reinterpret_cast<char*>(&vals[0]), 1);
	is.read(reinterpret_cast<char*>(&vals[1]), 1);
	is.read(reinterpret_cast<char*>(&vals[2]), 1);
	is.read(reinterpret_cast<char*>(&vals[3]), 1);
	val = (vals[3] << 24) +
		(vals[3] << 16) +
		(vals[3] << 8) +
		(vals[3] << 0);
	*/

	while (raw_read(is, val)) {
		v.push_back(val);
	}

	std::ofstream os(output_filename, std::ios::out);
	if (!os) {
		error("Cannot open file " + output_filename);
	}

	for (const auto& x : v) {
		os << x << '\n';
	}

	return EXIT_SUCCESS;
}

