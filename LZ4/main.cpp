#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

int read_header(std::ifstream& is, uint32_t& len_uncomp) {
	uint32_t mn, constant;
	is.read(reinterpret_cast<char*>(&mn), 4);
	is.read(reinterpret_cast<char*>(&len_uncomp), 4);
	is.read(reinterpret_cast<char*>(&constant), 4);
	if (mn != uint32_t(0x184C2103)) {
		return(EXIT_FAILURE);
	}
	if (constant != uint32_t(0x4D000000)) {
		return(EXIT_FAILURE);
	}
	return(EXIT_SUCCESS);
}

int read_sequence(std::ifstream& is, std::vector<uint8_t>& us, uint32_t& bl) {
	uint8_t token;
	is.read(reinterpret_cast<char*>(&token), 1);
	bl -= 1;
	uint64_t ll = 0;
	uint64_t ml = 0;
	ll = token >> 4;
	ml = token & 0b00001111;

	if (ll == 15) {
		uint8_t ext = 0;
		do {
			is.read(reinterpret_cast<char*>(&ext), 1);
			bl -= 1;
			ll += ext;
		} while (ext == 255);
	}
	if (ll != 0) {
		std::vector<uint8_t> lit(ll);
		is.read(reinterpret_cast<char*>(lit.data()), ll);
		bl -= ll;

		//write to uncompressed_stream
		for (const auto& x : lit) {
			us.push_back(x);
		}

		if (bl == 0) {
			return(EXIT_SUCCESS);
		}
	}

	uint16_t off;
	is.read(reinterpret_cast<char*>(&off), 2);
	bl -= 2;
	if (off == 0) {
		return(EXIT_FAILURE);
	}

	ml += 4;
	if (ml == 19) {
		uint8_t ext = 0;
		do {
			is.read(reinterpret_cast<char*>(&ext), 1);
			bl -= 1;
			ml += ext;
		} while (ext == 255);
	}

	//write to uncompressed-stream
	while (ml-- > 0) {
		us.push_back(us[us.size() - off]);
	}

	return(EXIT_SUCCESS);
}

int read_block(std::ifstream& is, std::ofstream& os, std::vector<uint8_t>& us, uint32_t& uncomp_len) {
	uint32_t bl;
	size_t size_curr = 0;
	is.read(reinterpret_cast<char*>(&bl), 4);
	while (bl > 0) {
		if (read_sequence(is, us, bl) != 0) {
			return(EXIT_FAILURE);
		}

		if ((us.size() / 65536) >= 2){
			int m = us.size() - 65537; //m dice quanti byte ho in più rispetto al necessario
			os.write(reinterpret_cast<const char*>(us.data()), m); //quindi scrivoi primi m su file
			us.erase(us.begin(), us.size() > m ? us.begin() + m : us.end()); //e rimuovo dallo zero alla zero + m elementi (perchè l'iterator last è escluso).
		}
		if ((us.size() - size_curr) > uncomp_len) {
			uncomp_len = 0;
		}
		else {
			uncomp_len -= (us.size() - size_curr);
		}
		size_curr = us.size();
	}
	return(EXIT_SUCCESS);
}

void write_fo_file(std::ofstream& os, std::vector<uint8_t>& us) {
	os.write(reinterpret_cast<const char*>(us.data()), us.size());
}

int decode(std::string ifile, std::string ofile) {
	std::ifstream is(ifile, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file." << std::endl;
		return(EXIT_FAILURE);
	}
	std::ofstream os(ofile, std::ios::binary);
	if (!os) {
		std::cout << "Error: cannot open output file." << std::endl;
		return(EXIT_FAILURE);
	}
	uint32_t uncomp_len;
	if (read_header(is, uncomp_len) != 0) {
		return(EXIT_FAILURE);
	}

	std::vector<uint8_t> us;
	while (uncomp_len > 0) {
		if (read_block(is, os, us, uncomp_len) != 0) {
			return(EXIT_FAILURE);
		}
	}
	write_fo_file(os, us);

	return(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Syntax Error." << std::endl;
		return(EXIT_FAILURE);
	}
	std::string ifile = argv[1];
	std::string ofile = argv[2];
	
	if (decode(ifile, ofile) != 0){
		return(EXIT_FAILURE);
	}
	return(EXIT_SUCCESS);
}