#include <fstream>

class bitwriter {
	std::ofstream& os_;
	uint8_t buffer_;
	uint8_t nbits_;

	void write_bit(uint8_t b) {
		if (nbits_ == 8) {
			nbits_ = 0;
			os_.write(reinterpret_cast<const char*>(&buffer_), 1);
			buffer_ = 0;
		}
		buffer_ = (buffer_ << 1) + b;
		++nbits_;
	}

	void flush() {
		if (nbits_ != 0 and nbits_ != 8) {
			buffer_ = buffer_ << (8 - nbits_);
			os_.write(reinterpret_cast<const char*>(&buffer_), 1);
		}
	}

public:
	bitwriter(std::ofstream& os) : os_(os), buffer_(0), nbits_(0) {}
	~bitwriter() { flush(); }

	void write(int u, int n) {
		while (n-- > 0) {
			write_bit((u >> n) & 0x01);
		}
	}
};


class bitreader {
	std::ifstream& is_;
	uint8_t buffer_;
	uint8_t nread_;

	uint8_t read_bit() {
		if (nread_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), 1);
			nread_ = 8;
		}
		--nread_;
		return ((buffer_ >> nread_) & 0x01);
	}

public:
	bitreader(std::ifstream& is) : is_(is), buffer_(0), nread_(0) {}
	~bitreader(){}

	void read(uint32_t& u, uint8_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) + read_bit();
		}
	}
};