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

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&num), size);
}

class bitwriter {
	std::ostream& os_;
	uint8_t buffer_;  //questo è il minimo di roba che posso scrivere in una volta sola (1 byte).
	size_t nbits_; //numero di bits che ho già letto, così so quando scrivere il byte.

	//
	//
	// +---+---+---+---+---+---+---+---+
	// | a | b | c | d | e | f | g | h |
	// +---+---+---+---+---+---+---+---+
	//
	// +---+---+---+---+---+---+---+---+
	// | b | c | d | e | f | g | h | 0 |
	// +---+---+---+---+---+---+---+---+
	//

	//se qualcuno mi fa cazzate e mi da in u un valore a più bits, devo essere sicuro di utilizzare solo il least significant.
	//uso l'AND:
	//0000.0010.0100.1101
	//         &
	//0000.0000.0000.0001 =
	//                  1

public:
	bitwriter(std::ostream& os) : os_(os), nbits_(0) { }

	~bitwriter() {
		flush();
	}
	
	std::ostream& write_bit(uint32_t u) {
		buffer_ = buffer_ * 2 + (u & 1);
		++nbits_;
		if (nbits_ == 8) {
			raw_write(os_, buffer_);
			nbits_ = 0;
		}
		return os_;
	}

	//write the n least significant bits of u, from the most significant to the least significant.
	std::ostream& write(uint32_t u, size_t n) {

		//for (size_t i = n - 1; i < n; --i) { //CAREFULL WITH DECREMENT!!! I DIVENTERA' POSITIVA INVECE CHE NEGATIVA NEL SOTTOZERO.
		while (n --> 0){ //while N goes to zero. Decrementare così equivale a decrementare N dopo il controllo.
			write_bit(u >> n);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, size_t n) {
		return write(u, n);
	}


	void flush(uint32_t u = 0) {
		while (nbits_ > 0) {
			write_bit(u);
		}
	}
};

int main(int argc, char* argv[]) {

	using std::cout;

	if (argc != 3) {
		error("SYNTAX:\n write_int11 <input_file.txt> <output_file.bin>\n");
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		error("Cannot open input file.\n");
	}

	std::ofstream os(argv[2]);
	if (!os) {
		error("Cannot open output file\n");
	}

	bitwriter bw(os);

	int32_t num;
	while (is >> num) {
		bw(num, 11);
	}

	return EXIT_SUCCESS;
}