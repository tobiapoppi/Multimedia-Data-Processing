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

	for (const auto& x : v) {
		os.write(reinterpret_cast<const char*>(&x), sizeof(x));
	}

	return EXIT_SUCCESS;
}

