#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <array>
#include <cstdint>
#include <iomanip>

void syntax() {
	std::cout << "SYNTAX:\n\n"
		<< "frequencies <input_file> <output_file> \n\n"
		<< "The program computes the frequency of every bytes of the input file.";
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

	std::string in_filename = argv[1];
	std::string out_filename = argv[2];

	std::ifstream is(in_filename, std::ios::binary); //I don't want to translate special characters
	if (!is) {
		error("Cannot open file " + in_filename);
	}

	std::array<int64_t, 256> count{ 0 }; //this is only the first number initialization. All the rest is 0.
										 // with arra.fill() i can fill with values, in an optimized way.
	


	while (true) {
		int val = is.get();
		if (val == EOF) {
			break;
		}
		count[val]++;
	}

	std::ofstream os(argv[2], std::ios::out);
	if (!os) {
		error("Cannot open file " + out_filename);
	}

	for (size_t i = 0; i < 256; ++i) {
		if (count[i] > 0) {
			os << std::hex
				<< std::setw(2)				//I specify that i want the width od output as 2
				<< std::setfill('0')		//I specify the char to fill the width added outputs
				<< i << std::dec << '\t' << count[i] << '\n';
		}
	}

	return EXIT_SUCCESS;
}