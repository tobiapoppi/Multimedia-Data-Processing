#include <iostream>
#include <vector>
#include <fstream>
#include <array>
#include <algorithm>

void syntax() {
	std::cout << "SYNTAX:\n\n"
		<< "huffman1 [c|d] <input_file> <output_file>\n\n";
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "ERROR: " << msg << "\n";
	exit(EXIT_FAILURE);
}

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	uint8_t buffer_;
	uint8_t n_bits_;
	std::istream& is_;

	bitreader(std::istream& is) : is_(is) {}

	int read_bit() {
		if (n_bits_ == 0) {
			if (!raw_read(is_, buffer_)) {
				return EOF;
			}
			n_bits_ = 8;
		}
		--n_bits_;
		return (buffer_ >> n_bits_) & 1;
	}

	std::istream& read(uint32_t& u, uint8_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}
};

int main(int argc, char* argv[]) {
	if (argc != 4) {
		syntax();
	}
	std::string input_filename = argv[2];
	std::string output_filename = argv[3];
	char* mode = argv[1];

	if (mode[0] == 'c') {

		std::ifstream is(input_filename, std::ios::binary);
		if (!is) {
			error("Cannot open file " + input_filename);
		}

		std::ofstream os(output_filename, std::ios::out);
		if (!is) {
			error("Cannot open file " + output_filename);
		}
		
		std::array<int64_t, 256> v{ 0 };
		is.seekg(0, std::ios::end);
		auto size = is.tellg();
		is.seekg(0, std::ios::beg);

		std::vector<uint8_t> v(size);
		is.read((char*)(v.data()), size);

		std::vector<std::pair<uint8_t, uint64_t>> count(256);
		std::generate(count.begin(), count.end(),
			[]() {
				static uint8_t i;
				return std::make_pair(i++, uint64_t(0));
			});
		for (const auto& x : v) {
			count[x].second++;
		}
		count.erase(remove_if(count.begin(), count.end(),
					[](const std::pair<uint8_t, uint64_t>& x) {
						return x.second == 0;
					}), count.end()
				);

		//count is now the set of symbols for Huffman, with their frequencies (probabilities)
		std::sort(count.begin(), count.end(), 
			[](const std::pair<uint8_t, uint64_t>& a, const std::pair<uint8_t, uint64_t>& b) {
				return (a.second < b.second);
			});

		//now we have to build the Huffman coding 

	}
	
}