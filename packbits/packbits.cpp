#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <queue>

void error(const char* msg) {
	std::cout << msg;
	exit(EXIT_FAILURE);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}
template<typename T>
std::ostream& raw_write(std::ostream& os, T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

bool cmp_map_items(std::pair<uint8_t, uint64_t>& a, std::pair<uint8_t, uint64_t>& b) {
	return a.second > b.second;
}

bool cmp_map_items_asc(std::pair<uint8_t, uint64_t>& a, std::pair<uint8_t, uint64_t>& b) {
	return a.second > b.second;
}

template<typename T1, typename T2>
void encode(std::istream& is, std::ostream& os) {
	uint8_t valcurr;
	uint8_t valnext;
	uint8_t val2next;
	uint8_t copy_count;
	uint8_t run_count;


	while (true) {
		copy_count = 0;
		run_count = 0;
		std::vector<uint8_t> v;

		if (is.peek() == EOF) {
			break;
		}
		raw_read(is, valcurr, 1);
		if (is.peek() == EOF) {
			break;
		}
		raw_read(is, valnext, 1);

		if (valcurr != valnext) {
			//copy command
			v.push_back(valcurr);
			do {
				valcurr = valnext;
				raw_read(is, valnext, 1);
				raw_read(is, val2next, 1);
				v.push_back(valcurr);
				++copy_count;
				is.seekg(-1, std::ios_base::cur);
			} while (valcurr != valnext && valnext != val2next && copy_count < 127 && is.peek() != EOF);

			//add 1 copy command with copy_count , vector
			raw_write(os, copy_count, 1);
			for (size_t i = 0; i < copy_count + 1; ++i) {
				raw_write(os, v[i], 1);
			}
			is.seekg(-1, std::ios_base::cur);
		}
		else {
			//run command
			++run_count;
			while (valcurr == valnext && run_count < 127 && is.peek() != EOF) {
				valcurr = valnext;
				raw_read(is, valnext, 1);
				++run_count;
			}
			//add 1 copy command with copy_count , vector
			uint16_t c = 257 - run_count;
			raw_write(os, c, 1);
			raw_write(os, valcurr, 1);
			//size_t pos = is.tellg() - 1;
			is.seekg(-1, std::ios_base::cur);
		}
	}
	uint8_t e = 128;
	raw_write(os, e, 1);
}

template<typename T1, typename T2>
void decode(std::istream& is, std::ostream& os) {
	while (is.peek != 128) {
		uint8_t val;
		val = is.get();
		if (1 <= val <= 127) {
			uint8_t copy_count = val + 1;
			for (size_t i = 0; i < copy_count; ++i) {

			}
		}
		else {
			uint8_t run_count = 257 - val;

		}
	}
	
}

int main(int argc, char* argv[]) {

	if (argc != 4) {
		error("SYNTAX: packbits [c|d] <input file> <output file>\n");
	}
	std::ifstream is(argv[2], std::ios::binary);
	if (!is) {
		error("Cannot open input file.\n");
	}

	std::ofstream os(argv[3], std::ios::binary);
	if (!os) {
		error("Cannot open output file.\n");
	}

	std::string mode = argv[1];

	if (mode == "c") {
		encode<uint8_t, uint64_t>(is, os);
	}
	else if (mode == "d") {
		decode<uint8_t, uint64_t>(is, os);
	}
	else {
		error("Syntax error: wrong mode parameter [c|d].");
	}

	return EXIT_SUCCESS;
}