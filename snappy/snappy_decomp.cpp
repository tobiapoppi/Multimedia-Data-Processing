#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>

uint32_t le2be_32(uint32_t n_l) {
	uint32_t a, b, c, d;
	a = n_l & 0b11111111;
	b = (n_l >> 8) & 0b11111111;
	c = (n_l >> 16) & 0b11111111;
	d = n_l >> 24;
	return a+b+c+d;
}

int main(int argc, char** argv) {
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::cout << "Error opening input file." << std::endl;
		return EXIT_FAILURE;
	}
	/*
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		std::cout << "Error opening output file." << std::endl;
		return EXIT_FAILURE;
	}
	*/

	//reading preamble

	uint8_t byte = is.get();
	std::vector<uint8_t> preamble;
	preamble.push_back(byte);
	while ((byte & 0b10000000) == 0b10000000) {
		//it means that we have another byte to read after this one
		byte = is.get();
		preamble.push_back(byte);
	}
	std::reverse(preamble.begin(), preamble.end());
	uint64_t file_length = 0;
	for (size_t i = 0; i < preamble.size(); ++i) {
		file_length = (file_length << 7) + (preamble[i] & 0b01111111);
	}
	std::cout << file_length; //funziona correttamente.

}