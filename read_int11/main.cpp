#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <iterator>
#include <algorithm>

void error(const char* msg) {
	std::cout << msg;
	exit(EXIT_FAILURE);
}

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
		while (n-- > 0) { //while N goes to zero. Decrementare così equivale a decrementare N dopo il controllo.
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

int8_t pow2(size_t nbits) {
	int8_t b = 0;
	for (size_t i = 0; i < nbits; ++i) {
		b += 2 ^ (i);
	}
	return b;
}

class bitreader {
	std::istream& is_;
	uint8_t buffer_;
	uint8_t nbits_;

public:
	bitreader(std::istream& is) : is_(is), nbits_(0) {}

	int read_bit() {
		if (nbits_ == 0) {
			if (!raw_read(is_, buffer_, 1)) {
				return EOF;
			}
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

	std::istream& read(uint32_t& b, size_t size) {
		b = 0;
		while (size --> 0) {
			b = (b << 1) | read_bit();
		}
		return is_;
	}

};

int main(int argc, char* argv[]) {

	using std::cout;

	if (argc != 3) {
		error("SYNTAX:\n read_int11 <input_file.bin> <output_file.txt>\n");
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		error("Cannot open input file.\n");
	}

	std::ofstream os(argv[2]);
	if (!os) {
		error("Cannot open output file\n");
	}

	bitreader br(is);
	uint32_t u;

	while (br.read(u, 11)) {
		int32_t n = u;
		if (n & 0x400) {	//se il risultato è 1 significa che il numero è rappresentato in complemento a due (negativo)
			n = n - 0x800;  //trasformo il numero negativo del complemento a due nella sua versione positiva
		}
		os << n << '\n';
	}

	return EXIT_SUCCESS;
}