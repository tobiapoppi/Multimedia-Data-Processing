#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cstring>

#define DEBUG

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

class bitreader {
	std::ifstream& is_;
	uint8_t buffer_;
	uint8_t nbits_;

	int read_bit() {
		if (nbits_ == 0) {
			nbits_ = 8;
			is_.read(reinterpret_cast<char*>(&buffer_), 1);
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

public:
	bitreader(std::ifstream& is) : is_(is), nbits_(0){}
	
	void read(uint32_t& u, uint8_t n){
		u = 0; //be sure that u starts from zero.
		while (n-- > 0) {
			int b = read_bit();
			u = (u << 1) + b;
		}
	}
};

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
	bitwriter(std::ofstream& os) : os_(os), nbits_(0), buffer_(0) {}
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
	std::unordered_map<uint64_t, std::string> entries;
	size_t maxb;
	size_t max_str_len;
	size_t max_i; //indice del dizionario più alto presente
	uint8_t limit;

	dict(uint8_t l) : maxb(0), max_str_len(0), max_i(0), limit(l){}

	void append(size_t i, uint8_t c) {
		++max_i;
		entries[max_i] = entries[i] + char(c);
		if ((entries[i].size() + 1) > max_str_len) {
			max_str_len = entries[i].size() + 1;
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
	void append(uint8_t c) {
		++max_i;
		entries[max_i] = char(c);
		if (max_str_len == 0) {
			max_str_len = 1;
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

	bool found = false;
	uint8_t nextc;
	
	while (is.peek() != EOF) {
		found = false;
		for (size_t l = 0; l < d.max_str_len; ++l) {
			if (found != true) {
				c.resize(d.max_str_len - l);
				auto pos = is.tellg();
				is.read(reinterpret_cast<char*>(&*c.begin()), (d.max_str_len - l));
				if (is.peek() == EOF) {
					is.clear();
					is.seekg(pos);
					continue;
				}
				for (size_t i = 1; i <= d.entries.size(); ++i) {
					auto f = d.entries.find(i);
					if (c == f->second && is.peek() != EOF) {
						is.read(reinterpret_cast<char*>(&nextc), 1);
#ifdef DEBUG
						std::cout << i << ", " << nextc << std::endl;
#endif // DEBUG

						bw.write(i, d.maxb);
						bw.write(nextc, 8);
						d.append(i, nextc);
						found = true;		
						break;
					}
				}
				if (!found) {
					is.seekg(pos);
				}
			}
		}
		if (found != true) {
			is.read(reinterpret_cast<char*>(&nextc), 1);
#ifdef DEBUG
			std::cout << "0, " << nextc << std::endl;
#endif // DEBUG

			if (d.maxb != 0) {
				//bw.write((d.max_i+1), d.maxb);
				bw.write(0, d.maxb);
			}
			bw.write(nextc, 8);
			d.append(nextc);
		}
	}
	return true;
}


int main(int argc, char** argv) {
	lz78encode(argv[1], argv[2], 10);
}
