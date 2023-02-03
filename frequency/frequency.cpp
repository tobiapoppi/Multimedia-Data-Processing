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

	void operator() (uint8_t val){
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

int main(int argc, char* argv[]) {

	using std::cout;
	
	if (argc != 3) {
		error("SYNTAX:\n frequencies <input file> <output file>\n");
	}
	
	std::ifstream is(argv[1]);
	if (!is) {
		error("Cannot open input file.\n");
	}
	is.unsetf(std::ios::skipws);

	/*
	while (is >> c) {
		++occurrencies[c];
	}*/

	frequency_counter f;
	
	/*
	for (std::istream_iterator<uint8_t> it(is); it != std::istream_iterator<uint8_t>(); ++it) {
		f(*it);
	}*/

	f = std::for_each(std::istream_iterator<uint8_t>(is), std::istream_iterator<uint8_t>(), f);
	// for each from first iterator to second iterator call the function f (call the ()operator of functor f).
	//se non assegni ad f il valore di ritorno di foreach non rimane salvato, perchè la f passata nei parametri è una copia.

	std::ofstream os(argv[2]);
	if (!os) {
		error("Cannot open output file\n");
	}
	
	for (size_t i = 0; i < 256; ++i) {
		if (f[i] != 0) {
			os << std::setfill('0') << std::setw(2) << std::hex << i;
			os << std::dec << '\t' << f[i] << '\n';
			// same as c printf specifier --> "%02x" x = hexadecimal; 2 = 2 characters per number; 0 = filled with zeros. 
		}
	}

	cout << "Entropy: " << f.entropy() << '\n';

	return EXIT_SUCCESS;
}