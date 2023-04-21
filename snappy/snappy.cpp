#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>

uint32_t le2be(uint32_t n_l, size_t nbytes) {
	uint32_t n = 0;
	n = n + (n_l & 0b11111111);
	std::cout << "n_l" << n_l << std::endl;
	std::cout << "n" << n << std::endl;
	if (nbytes > 0){
		uint8_t shifter = 8;
		for (size_t i = 0; i < nbytes - 1; ++i) {
			n = (n << 8) + ((n_l >> shifter) & 0b11111111);
			shifter += 8;
			std::cout << "n_l" << n_l << std::endl;
			std::cout << "n" << n << std::endl;
		}
		return n;
	}
	std::cout << "Number of bytes <= 0 during little endian to big endian is not acceptable." << std::endl;
	return EXIT_FAILURE;
}

void read_literal(uint8_t first_byte, std::ifstream& is, std::fstream& os) {
	uint8_t msb_6 = 0;
	msb_6 = first_byte >> 2;
	uint32_t l = 0;

	if (msb_6 < 60) {
		l = msb_6 + 1;
	}
	else if (msb_6 == 60) {
		//read next byte
		l = is.get() + 1;
	}
	else if (msb_6 == 61) {
		//read next 2 bytes
		is.read(reinterpret_cast<char*>(&l), 2);
		//l = le2be(l, 2) + 1;
		l += 1;
	}
	else if (msb_6 == 62) {
		//read next 3 bytes
		is.read(reinterpret_cast<char*>(&l), 3);
		//l = le2be(l, 3) + 1;
		l += 1;
	}
	else {
		//read next 4 bytes
		is.read(reinterpret_cast<char*>(&l), 4);
		//l = le2be(l, 4) + 1;
		l += 1;
	}
	// read literal data and write it to output file
	std::vector<uint8_t> lit(l);
	is.read(reinterpret_cast<char*>(lit.data()), l);
	os.write(reinterpret_cast<const char*>(lit.data()), l);
}

bool copy_bytes(uint8_t first_byte, std::ifstream& is, std::fstream& os, uint8_t mode) {
	uint8_t l = 0;
	uint32_t offset = 0;
	if (mode == 1) {
		l = ((first_byte & 0b00011100) >> 2) + 4;
		offset = ((first_byte & 0b11100000) << 3) + is.get();
	}
	else if (mode == 2) {
		l = ((first_byte & 0b11111100) >> 2) + 1;
		is.read(reinterpret_cast<char*>(&offset), 2);
		//offset = le2be(offset, 2);
	}
	else if (mode == 4) {
		l = ((first_byte & 0b11111100) >> 2) + 1;
		is.read(reinterpret_cast<char*>(&offset), 4);
		//offset = le2be(offset, 4);
	}
	else {
		std::cout << "Only Copy with 1-byte offset, Copy with 2-byte offset, Copy with 4-byte offset are accapted." << std::endl;
		return false;
	}

	//execute actual copy (build vector)
	int64_t signed_offset;
	signed_offset = int64_t(offset);
	std::vector<uint8_t> cp(l);
	auto pos = os.tellg();
	os.seekg(-signed_offset, std::ios_base::cur);

	if (l <= offset) {
		os.read(reinterpret_cast<char*>(cp.data()), l);
	}
	else {
		size_t i = 0;
		while (1) {
			if (l >= offset) {
				os.read(reinterpret_cast<char*>(&cp[i]), offset);
				l -= offset;
				i += offset;
				os.seekg(pos);
				os.seekg(-signed_offset, std::ios_base::cur);
			}
			else {
				os.read(reinterpret_cast<char*>(&cp[i]), l);
				break;
			}
		}
	}
	os.seekg(pos);

	// write copy vector to output file
	os.write(reinterpret_cast<const char*>(cp.data()), cp.size());

	return true;
}

int main(int argc, char** argv) {
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::cout << "Error opening input file." << std::endl;
		return EXIT_FAILURE;
	}
	
	std::ofstream create_file(argv[2], std::ios::binary);
	create_file.close();


	std::fstream os(argv[2], std::ios::in | std::ios::out | std::ios::binary);
	if (!os) {
		std::cout << "Error opening output file." << std::endl;
		return EXIT_FAILURE;
	}
	

	//reading preamble

	uint8_t byte;
	byte = is.get();
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
	//std::cout << file_length; //funziona correttamente.

	//read first literal (length)

	uint8_t b;
	is.read(reinterpret_cast<char*>(&b), 1);
	uint8_t tag;
	tag = b & 0b00000011;
	if (tag != 0) {
		std::cout << "Error, compressed stream must start with literal!" << std::endl;
		return EXIT_FAILURE;
	}

	read_literal(b, is, os);
	while (is.peek() != EOF) {
		b = is.get();
		tag = b & 0b00000011;
		if (tag == 0) {
			read_literal(b, is, os);
		}
		else if (tag == 1) {
			if (!copy_bytes(b, is, os, 1)) {
				std::cout << "Error during copy 1-bytes." << std::endl;
				return EXIT_FAILURE;
			}
		}
		else if (tag == 2) {
			if (!copy_bytes(b, is, os, 2)) {
				std::cout << "Error during copy 2-bytes." << std::endl;
				return EXIT_FAILURE;
			}
		}
		else if (tag == 3) {
			if (!copy_bytes(b, is, os, 4)) {
				std::cout << "Error during copy 4-bytes." << std::endl;
				return EXIT_FAILURE;
			}
		}
		else { 
			return EXIT_FAILURE; 
		}
	}
}