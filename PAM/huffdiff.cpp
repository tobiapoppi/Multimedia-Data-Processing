#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <vector>

using vec3b = std::array<uint8_t, 3>;

template<typename T>
class mat {
	int rows_ = 0, cols_ = 0;
	std::vector<T> data_;
public:
	mat() {}
	mat(int rows, int cols) : rows_(rows), cols_(cols), data_(rows* cols) {}
	int rows() const { return rows_; }
	int cols() const { return cols_; }
	int size() const { return data_.size(); }

	T& operator() (int r, int c) {
		return data_[r * cols_ + c];
	}
	const T& operator() (int r, int c) const {
		return data_[r * cols_ + c];
	}
	char* rawdata() { return reinterpret_cast<char*>(&data_[0]); }
	const char* rawdata() const { return reinterpret_cast<const char*>(&data_[0]); }
	int rawsize() const { return rows_ * cols_ * sizeof(T); }
	auto begin() { return data_.begin(); }
	auto begin() const { return data_.begin(); }
	auto end() { return data_.end(); }
	auto end() const { return data_.end(); }
};

struct frequencies {
	std::unordered_map<uint8_t, uint32_t> f_;

	void operator()(const uint8_t& sym) {
		++f_[sym];
	}

	auto& operator[](const uint8_t& sym) {
		return f_[sym];
	}
};

struct bitwriter {
	uint8_t buffer_;
	uint8_t nbits_;
	std::ofstream& os_;

	bitwriter(std::ofstream& os) : buffer_(0), nbits_(0), os_(os) {}
	~bitwriter() {
		flush();
	}
	void flush() {
		while (nbits_ > 0) {
			write_bit(0);
		}
	}

	void write(uint32_t u, uint8_t n) {
		while (n-- > 0) {
			write_bit(u >> n);
		}
	}
	
	void write_bit(uint32_t u) {
		buffer_ = (buffer_ << 1) + u;
		++nbits_;
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<const char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

};

struct huffman {

	struct node {
		uint8_t sym_;
		size_t prob_;
		node* left_ = nullptr;
		node* right_ = nullptr;

		node(const uint8_t& sym, size_t p) : sym_(sym), prob_(p) {}

		node(node* n1, node* n2) : prob_(n1->prob_ + n2->prob_), left_(n1), right_(n2) {}
	};

	struct pnode {
		node* p_;
		pnode(node* p) : p_(p){}
		operator node* () {
			return p_;
		}

		//with this compare operator I can automatically sort nodes from the one with higher probability (frequency) to the lower one (least probable).
		bool operator<(const pnode& rhs) const {
			return p_->prob_ > rhs.p_->prob_;
		}
	};

	struct code {
		uint8_t sym_;
		uint32_t len_, val_;
		bool operator<(const code& rhs) const {
			return len_ < rhs.len_;
		}
	};

	void create_table(const std::unordered_map<uint8_t, uint32_t> map) {
		std::vector<pnode> v;
		std::vector<std::unique_ptr<node>> storage;
		for (auto& x : map) {
			node* n = new node(x.first, x.second);
			v.push_back(n);
			storage.emplace_back(n);
		}
		std::sort(v.begin(), v.end());
		while (v.size() > 1) {
			//I take the two least probable nodes
			pnode first = v.back();
			v.pop_back();
			pnode second = v.back();
			v.pop_back();
			pnode n = new node(first, second);
			
			storage.emplace_back(n);
			//I get the position where to insert the node between the value itself and the lower one (lower bound);
			auto it = std::lower_bound(v.begin(), v.end(), n);
			v.insert(it, n);
		}
		node* root = v.back();
		compute_len(root, 0);
		std::sort(codes_.begin(), codes_.end());
		compute_zero_canonical();
	}

	void compute_len(const node* p, uint32_t len) {
		if (p->left_ == nullptr) {
			code c = { p->sym_, len, 0 }; //I put 0 inside val because I'll calculate them through canonical
			codes_.push_back(c);
		}
		else {
			compute_len(p->left_, len + 1);
			compute_len(p->right_, len + 1);
		}
	}

	void compute_zero_canonical() {
		code curr = { ' ', 0, 0 };
		for (auto& x : codes_) {
			while (curr < x) {
				++curr.len_;
				curr.val_ = curr.val_ << 1;
			}
			x.val_ = curr.val_;
			++curr.val_;
		}
	}

	std::vector<code> codes_;

};

void compress(std::string ifile, std::string ofile) {
	std::ifstream is(ifile, std::ios::binary);
	if (!is) {
		std::cout << "Error: cannot open input file." << std::endl;
		exit(EXIT_FAILURE);
	}
	std::ofstream os(ofile, std::ios::binary);
	if (!os) {
		std::cout << "Error: cannot open output file." << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string s;
	uint32_t h = 0;
	uint32_t w = 0;
	uint8_t d = 0;
	uint32_t mv = 0;
	std::string tupletype;

	is >> s;
	char consume;
	consume = is.get();
	while (s != "ENDHDR") {
		is >> s;
		if (s.front() == '#') {
			std::getline(is, s);
		}
		if (s == "HEIGHT") {
			is >> h;
		}
		else if (s == "WIDTH") {
			is >> w;
		}
		else if (s == "DEPTH") {
			is >> d;
		}
		else if (s == "MAXVAL") {
			is >> mv;
		}
		else if (s == "TUPLTYPE") {
			is >> s;
			tupletype += s;
		}
	}
	is.get();

	//read image data
	mat<uint8_t> I(h, w);
	is.read(reinterpret_cast<char*>(I.rawdata()), I.rawsize());

	//calculate difference matrix
	mat<uint16_t> D(h, w);
	D(0, 0) = I(0, 0);
	for (size_t y = 0; y < I.rows(); ++y) {
		for (size_t x = 0; x < I.cols(); ++x) {
			if (x == 0 && y > 0) {
				D(y, 0) = I(y, 0) - I(y - 1, 0);
			}
			else if (x > 0) {
				D(y, x) = I(y, x) - I(y, x - 1);
			}
		}
	}

	//calculate frequencies of symbols
	frequencies f;
	f = std::for_each(D.begin(), D.end(), f);

	huffman huff;
	huff.create_table(f.f_);

	os << "HUFFDIFF";
	os.write(reinterpret_cast<const char*>(&w), 4);
	os.write(reinterpret_cast<const char*>(&h), 4);

	bitwriter bw(os);
	bw.write(huff.codes_.size(), 9);

	for (auto& x : huff.codes_) {
		bw.write(x.sym_, 9);
		bw.write(x.len_, 5);
	}

}

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cout << "Wrong syntax." << std::endl;
		return EXIT_FAILURE;
	}
	std::string ifile = argv[2];
	std::string ofile = argv[3];

	if (argv[1][0] == 'c') {
		compress(ifile, ofile);
	}

	/*else if (char(argv[1]) == 'd') {
		decompress();
	}*/

	else {
		std::cout << "Wrong syntax." << std::endl;
		return EXIT_FAILURE;
	}
}