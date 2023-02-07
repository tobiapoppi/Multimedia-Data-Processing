#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <list>
#include <regex>
#include <sstream>
#include <memory>
#include <iomanip>

void error(const char* msg) {
	std::cout << msg;
	exit(EXIT_FAILURE);
}

class Item {
protected:
	static bool ReadUpToX(std::istream& is, std::string& data, char x) {
		data.clear();
		while (is.peek() != EOF && is.peek() != x) {
			data += is.get();
		}
		return is && is.peek() == x;
	}
	static void ReadExpectedCharacter(std::istream& is, char exp) {
		using namespace std::literals;
		int c = is.get();
		if (c != exp) {
			throw std::logic_error("Expected character "s + exp + ", but got " + static_cast<char>(c));
		}
	}
public:
	static std::unique_ptr<Item> Decode(std::istream& is, const std::string& key = "");
	virtual std::ostream& Print(std::ostream& os, int tab) const = 0;
	virtual ~Item() = default;
};
class StringItem : public Item {
protected:
	std::string string_val_;
public:
	StringItem(std::istream& is) {
		std::string ascii_string_len;
		if (!ReadUpToX(is, ascii_string_len, ':')) {
			throw std::logic_error("Unable to read string length.\n");
		}

		std::string::size_type string_len;
		std::stringstream ss(ascii_string_len);
		ss >> std::dec >> string_len;

		ReadExpectedCharacter(is, ':');

		string_val_.resize(string_len);
		is.read(&string_val_[0], string_len);
		if (is.gcount() != string_len) {
			throw std::logic_error("Unable to read the entire string.\n");
		}
	}

	bool operator<(const StringItem& rhs) const {
		return string_val_ < rhs.string_val_;
	}

	std::ostream& Print(std::ostream& os, int tab) const {
		os << '"';
		for (const auto& c : string_val_) {
			os << (c < 32 || c > 126 ? '.' : c);
		}
		os << '"';
		return os;
	}

	operator std::string() const {
		return string_val_;
	}
};
class PiecesItem : public StringItem {
public:
	PiecesItem(std::istream& is) : StringItem(is) {}

	std::ostream& Print(std::ostream& os, int tab) const {
		using namespace std;
		for (size_t i = 0; i < string_val_.size(); i += 20) {
			os << '\n' << std::string(tab + 1, '\t');
			for (size_t j = 0; j < 20; ++j) {
				os << hex << setw(2) << setfill('0') << unsigned char(string_val_[i + j]);
			}
		}
		return os;
	}
};
class IntegerItem : public Item {
	int64_t val_;
public:
	IntegerItem(std::istream& is) {
		std::string encoded_int;
		if (!ReadUpToX(is, encoded_int, 'e')) {
			throw std::logic_error("Unable to read integer item.\n");
		}

		std::regex r("i([-+]?(0|[1-9][0-9]*))");
		std::smatch m;

		if (!std::regex_match(encoded_int, m, r)) {
			throw std::logic_error("Wrong integer format.\n");
		}
		ReadExpectedCharacter(is, 'e');

		std::stringstream ss(m[1].str());
		ss >> std::dec >> val_;
	}

	std::ostream& Print(std::ostream& os, int tab) const {
		os << val_;
		return os;
	}
};


class ListItem : public Item {
	std::list<std::unique_ptr<Item>> values_; //This is why we need a base class. We need to insert
	//into this list elements which we don't know their type.
public:
	ListItem(std::istream& is) {
		ReadExpectedCharacter(is, 'l');
		while (is && is.peek() != 'e') {
			values_.push_back(Decode(is));
		}
		ReadExpectedCharacter(is, 'e');
	}

	std::ostream& Print(std::ostream& os, int tab) const {
		os << "[\n";
		tab++;
		for (const auto& e : values_) {
			os << std::string(tab, '\t');
			e->Print(os, tab);
			os << '\n';
		}
		os << std::string(tab - 1, '\t') << ']';
		return os;
	}
};

class DictItem : public Item {
	std::map<StringItem, std::unique_ptr<Item>> d_values_; //This is why we need a base class. We need to insert
	//into this list elements which we don't know their type.

public:
	DictItem(std::istream& is) {
		ReadExpectedCharacter(is, 'd');
		while (is && is.peek() != 'e') {
			StringItem key(is);
			d_values_[key] = Decode(is, std::string(key));
		}
		ReadExpectedCharacter(is, 'e');
	}

	std::ostream& Print(std::ostream& os, int tab) const {
		os << "{\n";
		tab++;
		for (const auto& c : d_values_) {
			os << std::string(tab, '\t');
			c.first.Print(os, 0);
			os << " => ";
			c.second->Print(os, tab);
			os << '\n';
		}
		os << std::string(tab - 1, '\t') << '}';
		return os;
	}
};



std::unique_ptr<Item> Item::Decode(std::istream& is, const std::string& key) {
	switch (is.peek()) {
	case 'd':
		return std::make_unique<DictItem>(is);
	case 'l':
		return std::make_unique<ListItem>(is);
	case 'i':
		return std::make_unique<IntegerItem>(is);
	default:
		if (key == "pieces") {
			return std::make_unique<PiecesItem>(is);
		}
		else {
			return std::make_unique<StringItem>(is);
		}
	}
}



int main(int argc, char** argv) {

	if (argc != 2) {
		error("Wrong parameter list: torrent <inputfile.torrent>.\n");
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		error("Cannot open input file.\n");
	}

	auto init_ptr = Item::Decode(is);
	init_ptr->Print(std::cout, 0);


	return EXIT_SUCCESS;
}