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
										 // with array.fill() i can fill with values, in an optimized way.
	
	is.seekg(0, std::ios::end);
	auto size = is.tellg();
	is.seekg(0, std::ios::beg);

	std::vector<uint8_t> v(size);
	is.read();

	while (true) {
		int val = is.get();
		if (val == EOF) {
			break;
		}
		count[val]++;
	}

	std::vector<std::pair<uint8_t, uint64_t>> count(256);
	std::generate(count.begin(), count.end(),
		[]() {
			static uint8_t i;
			return std::make_pair(i++, uint64_t(0));
		}
		);
	
	for (const auto& x : v) {
		count[x].second++;
	}
	count.erase(remove_if(count.begin(), count.end(),
		[](const std::pair<uint8_t, uint64_t>& t))


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