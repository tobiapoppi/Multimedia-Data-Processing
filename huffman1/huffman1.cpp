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

struct bitwriter {
	uint8_t buffer_ = 0;
	size_t nwrotebits_ = 0;
	std::ostream& os_;

	bitwriter(std::ostream& os, bool mode = 0) : os_(os) {}
	~bitwriter() {
		flush();
	}

	void write_bit(uint64_t u) {
		buffer_ = (buffer_ << 1) + (u & 1);
		++nwrotebits_;
	}

	void operator()(uint64_t num, uint8_t n) {
		while (n-- > 0) {
			if (nwrotebits_ == 8) {
				raw_write(os_, buffer_, 1);
				nwrotebits_ = 0;
			}
			write_bit(num >> n);
		}
	}

	void flush(uint8_t fill = 0) {
		while (nwrotebits_ != 8) {
			write_bit(0);
		}
		raw_write(os_, buffer_, 1);
	}
};

struct bitreader {
	uint8_t buffer_;
	size_t nreadbits_ = 8;
	std::istream& is_;

	bitreader(std::istream& is) : is_(is) {	}
	
	uint64_t read_bit() {
		uint64_t b = (buffer_ >> (8 - nreadbits_ - 1)) & 1;
		++nreadbits_;
		return b;
	}

	void operator() (uint64_t& val, uint64_t n) {
		val = 0;
		for (size_t i = 0; i < n; ++i) {
			if (nreadbits_ == 8) {
				raw_read(is_, buffer_, 1);
				nreadbits_ = 0;
			}
			val = (val << 1) + (read_bit() & 1);
		}
	}

};

struct huffman {
	struct node {
		uint8_t sym_;
		uint64_t count_;
		uint64_t code_ = 0;
		uint8_t len_ = 0;
		node* left_ = nullptr;
		node* right_ = nullptr;
	
		node(const std::pair<uint8_t, uint64_t>& n) : sym_(n.first), count_(n.second){ }

		node(node* l, node* r) : left_(l), right_(r), count_(l->count_ + r->count_) { }

		friend bool operator<(const node& lhs, const node& rhs) {
			return lhs.count_ > rhs.count_;
		}
	};

	std::vector<std::unique_ptr<node>> nodes_;
	std::unordered_map<uint8_t, node*> symbols_;

	struct comparator {
		bool operator() (const node* lhs, const node* rhs) {
			return *lhs < *rhs;
		}
	};

	node* make_node(std::pair<uint8_t, uint64_t>& n) {
		nodes_.emplace_back(std::make_unique<node>(n));
		return nodes_.back().get();
	}

	node* make_node(node* a, node* b) {
		nodes_.emplace_back(std::make_unique<node>(a, b));
		return nodes_.back().get();
	}

	void make_codes(node* n, uint8_t len = 0, int64_t code = 0) {
		if (n->left_ == nullptr) {
			n->len_ = len;
			n->code_ = code;
			symbols_[n->sym_] = n;
		}
		else {
			make_codes(n->left_, len + 1, code << 1);
			make_codes(n->right_, len + 1, (code << 1) + 1);
		}
	}

	template<class _InIt>
	huffman(_InIt _First, _InIt _Last) {
		std::priority_queue<node*, std::vector<node*>, comparator> pq;

		while (_First != _Last) {
			pq.push(make_node(* _First));
			++_First;
		}

		while (pq.size() > 1) {
			auto a = pq.top();
			pq.pop();
			auto b = pq.top();
			pq.pop();
			pq.push(make_node(a, b));
		}
		auto root = pq.top();
		pq.pop();

		make_codes(root);
	}

	huffman(std::tuple<uint8_t, uint32_t, uint64_t> t) {

	}

	node* operator[] (uint8_t sym) {
		return symbols_[sym];
	}
};

template<typename T1, typename T2>
void encode(std::vector<std::pair<T1, T2>>& v, std::istream& is,std::ostream& os, uint32_t c) {

	//ENCODE
	huffman h(v.begin(), v.end());

	is.clear();
	is.seekg(0);

	//magic number
	raw_write(os, "HUFFMAN1", 8);

	//table_entries (number of items in the H table)
	uint8_t entries = static_cast<uint8_t>(h.symbols_.size());
	if (entries == 256) {
		entries = 0;
	}
	raw_write(os, entries, 1);

	//huffman_table
	bitwriter bw(os);
	for (auto& x : h.symbols_) {
		bw(x.first, 8);
		bw(x.second->len_, 5);
		bw(x.second->code_, x.second->len_);
	}
	bw(c, 32);

	//write data with h_encoding.
	uint8_t val;
	while (raw_read(is, val, 1)) {
		bw(h.symbols_[val]->code_, h.symbols_[val]->len_);
	}
}

template<typename T1, typename T2>
void decode(std::istream& is, std::ostream& os) {
	
	//read magic number
	std::string mn(8, ' ');
	raw_read(is, mn[0], 8);
	if (mn != "HUFFMAN1") {
		error("Wrong input format.\n");
	}

	//table_entries (number of items in the H table)
	uint16_t entries = is.get();
	if (entries == 0) {
		entries = 256;
	}

	struct triplet {
		uint64_t sym_;
		uint64_t len_;
		uint64_t code_;

		bool operator<(const triplet& rhs) {
			return len_ < rhs.len_;
		}
	};

	//huffman_table
	bitreader br(is);
	std::vector<triplet> vh;

	for (size_t i = 0; i < entries; ++i) {
		triplet t;
		br(t.sym_, 8);
		br(t.len_, 5);
		br(t.code_, t.len_);
		vh.push_back(t);
	}

	//num_symbols in file (in big endian)
	uint64_t tot;
	uint64_t buff[4];
	br(buff[0], 8);
	br(buff[1], 8);
	br(buff[2], 8);
	br(buff[3], 8);
	tot = (uint32_t)buff[3] | (uint32_t)buff[2] << 8 | (uint32_t)buff[1] << 16 | (uint32_t)buff[0] << 24;
	
	//write data with h_encoding.
	std::sort(vh.begin(), vh.end());
	
	for (size_t i = 0; i < tot; ++i) {
		uint32_t len = 0, code = 0;
		size_t pos = 0;
		while (pos < vh.size()) {
			while (vh[pos].len_ > len) {
				uint64_t bit;
				br(bit, 1);
				code = (code << 1) | bit;
				++len;
			}
			if (vh[pos].code_ == code) {
				break;
			}
			++pos;
		}
		if (pos == vh.size()) {
			error("Bad encoding mistakes.\n");
		}
		raw_write(os, vh[pos].sym_, 1);
	}
}

int main(int argc, char* argv[]) {

	if (argc != 4) {
		error("SYNTAX: huffman1 [c|d] <input file> <output file>\n");
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
		uint8_t val;
		std::vector<std::pair<uint8_t, uint64_t>> v(256, { 0, 0 });
		uint32_t all_counter = 0;

		while (raw_read(is, val, 1)) {
			v[val].first = val;
			++v[val].second;
			++all_counter;
		}

		std::sort(v.begin(), v.end(), cmp_map_items);

		for (size_t i = 0; i < v.size(); ++i) {
			if (v[i].second == 0) {
				v.erase(v.begin() + i, v.end());
				break;
			}
		}
		encode<uint8_t, uint64_t>(v, is, os, all_counter);
	}
	else if (mode == "d") {
		decode<uint8_t, uint64_t>(is, os);
	}
	else {
		error("Syntax error: wrong mode parameter [c|d].");
	}
	
	return EXIT_SUCCESS;
}