#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cstring>

//#define DEBUG

void error(const char* msg) {
	std::cout << msg;
}

uint8_t get_numbits(uint32_t dictlen) {
	uint8_t n = 0;
	while (dictlen / 2 != 0) {
		++n;
		dictlen = dictlen / 2;
	}
	return n + 1;
}

class bitwriter {
	std::ofstream& os_;
	uint8_t buffer_;
	uint8_t nbits_;

	void write_bit(uint32_t& u) {
		u = u & 1;
		
		buffer_ = (buffer_ << 1) + u;
		++nbits_;
		
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<const char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

public:
	bitwriter(std::ofstream& os) : os_(os), buffer_(0), nbits_(0) {}
	~bitwriter() {
		flush();
	}
	void flush() {
		if (nbits_ != 0) {
			while (nbits_ != 8) {
				buffer_ = (buffer_ << 1);
				++nbits_;
			}
			os_.write(reinterpret_cast<const char*>(&buffer_), 1);
		}
	}

	void write(uint32_t u, uint8_t n) {
		while (n-- > 0) {
			uint32_t b = (u >> n) & 1;
			write_bit(b);
		}
	}
};


struct dict {
	std::unordered_map<std::string, uint64_t> entries;
	size_t maxb;
	size_t max_str_len;
	size_t max_i; //indice del dizionario più alto presente
	uint8_t limit;

	dict(uint8_t l) : maxb(0), max_str_len(0), max_i(0), limit(l){}

	void append(std::string s) {
		++max_i;
		std::pair<std::string, uint64_t> element = { s, max_i };
		entries.insert(element);
		if (s.size() > max_str_len) {
			max_str_len = s.size();
		}
		maxb = get_numbits(max_i);
		if (maxb > limit) {
			entries.clear();
			max_i = 0;
			max_str_len = 0;
			maxb = 0;
#ifdef DEBUG
			std::cout << "clear_dictionary" << std::endl;
#endif // DEBUG
		}
	}
};

bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is) {
		error("Cannot open input file.");
		return false;
	}
	
	std::ofstream os(output_filename, std::ios::binary);
	if (!os) {
		error("Cannot open output file.");
		return false;
	}

	//write magic number
	char mn[4] = { 'L','Z','7','8'};
	os.write(reinterpret_cast<const char*>(&mn), 4);

	bitwriter bw(os);
	bw.write(maxbits, 5);

	std::string c = "";
	dict d(maxbits);

	uint64_t i = 1;

	while (true) {
		char c = is.get();
		int n = 0;
		if (c == EOF) {
			break;
		}
		std::string s;
		s.push_back(c);
		auto p = d.entries.find(s);
		while (p != d.entries.end() && is.peek() != EOF) {
			n = p->second;
			c = is.get();
			s.push_back(c);
			p = d.entries.find(s);
		}
		if (d.maxb != 0) {
			bw.write(n, d.maxb);
		}
		bw.write(c, 8);
		d.append(s); //appendo specificando nuova stringa
		++i;
	}
	return true;
}

/*
int main(int argc, char** argv) {
	lz78encode(argv[1], argv[2], 10);
}
*/
