#include <fstream>
#include <iostream>
#include <array>
#include <iomanip>
#include <iterator>
#include <algorithm>

void error(const char* msg) {
	std::cout << msg;
	exit(EXIT_FAILURE);
}

struct frequency_counter {
	std::array<size_t, 256> occurrencies;

	frequency_counter() : occurrencies{ 0 } {}

	void operator() (uint8_t val) {
		++occurrencies[val];
	}

	size_t& operator[] (uint8_t pos) {
		return occurrencies[pos];
	}

	const size_t& operator[] (uint8_t pos) const {
		return occurrencies[pos];
	}

	double entropy() {
		double tot = 0.0;
		for (const auto& x : occurrencies) {
			tot += x;
		}
		double H = 0.0;
		for (const auto& x : occurrencies) {
			if (x != 0) {
				double px = x / tot;
				H += px * log2(px);
			}
		}
		return -H;
	}

};

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&num), size);
}

int main(int argc, char* argv[]) {

	using std::cout;

	if (argc != 3) {
		error("SYNTAX:\n write_int32 <input_file.txt> <output_file.bin>\n");
	}

	std::ifstream is(argv[1]);
	if (!is) {
		error("Cannot open input file.\n");
	}

	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		error("Cannot open output file\n");
	}
	int32_t num;
	while (is >> num) {
		raw_write(os, num);
	}

	return EXIT_SUCCESS;
}