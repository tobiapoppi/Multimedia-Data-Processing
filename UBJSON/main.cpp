#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <functional>
#include <exception>
#include <map>

#include "ppm.h"
#include "image.h"

struct elem {
	const uint8_t type_ = 0;
	int8_t data_ = 0;
	elem(){}
	virtual ~elem(){}
};

struct rgb {
	uint8_t R_, G_, B_;
};

struct canvas {
	int w_, h_, x_off, y_off;
	rgb background_;
};


struct null_type : elem {
	const uint8_t type_ = 'Z';
	null_type() {}
};
struct noop_type : elem {
	const uint8_t type_ = 'N';
	noop_type() {}
};
struct true_type : elem {
	const uint8_t type_ = 'T';
	true_type() {}
};
struct false_type : elem {
	const uint8_t type_ = 'F';
	false_type() {}
};
struct int8_type : elem {
	const uint8_t type_ = 'i';
	int8_t data_;
	int8_type() {}
};
struct uint8_type : elem {
	const uint8_t type_ = 'U';
	uint8_t data_;
	uint8_type() {}
};
struct int16_type : elem {
	const uint8_t type_ = 'I';
	int16_t data_;
	int16_type() {}
};
struct int32_type : elem {
	const uint8_t type_ = 'l';
	int32_t data_;
	int32_type() {}
};
struct float32_type : elem {
	const uint8_t type_ = 'd';
	float data_;
	float32_type() {}
};
struct float64_type : elem {
	const uint8_t type_ = 'D';
	double data_;
	float64_type() {}
};
struct char_type : elem {
	const uint8_t type_ = 'C';
	uint8_t data_;
	char_type() {}
};
struct string_type : elem {
	const uint8_t type_ = 'S';
	elem* len_;
	std::string data_;
	string_type() {}
};

struct object_type : elem {
	elem* len_ = nullptr;
	std::map<std::string, elem*> map_;
	object_type() {}
};
struct array_type : elem {
	elem* len_ = nullptr;
	std::vector<uint8_t> arrdata_;
	uint8_t dtype_;
	array_type() {}
};

std::pair<std::string, elem*> read_couple(std::ifstream& is);

void le2be(uint32_t& u, uint8_t nbytes) {
	int32_t n = 0;
	switch (nbytes) {
	case 2:
		u = u & 0x0000FFFF;
		n = (u >> 8) + ((u << 8) & 0x0000FF00);
	case 4:
		n = (u >> 24) + ((u >> 8) & 0x0000FF00) + ((u << 8) & 0x00FF0000) + ((u << 24) & 0xFF000000);
	}
	u = n;
}

void le2be(int16_t& u, uint8_t nbytes) {
	int32_t n = 0;
	switch (nbytes) {
	case 2:
		u = u & 0x0000FFFF;
		n = (u >> 8) + ((u << 8) & 0x0000FF00);
	case 4:
		n = (u >> 24) + ((u >> 8) & 0x0000FF00) + ((u << 8) & 0x00FF0000) + ((u << 24) & 0xFF000000);
	}
	u = n;
}



void read_key(std::ifstream& is, std::string& s) {
	uint8_t len = is.get();
	is.read(reinterpret_cast<char*>(&s[0]), len);
}

elem* read_val(std::ifstream& is, uint8_t carattere = 0, bool read = true) {
	uint8_t c;
	if (read) {
		c = is.get();
	}
	else {
		c = carattere;
	}
	elem* pointer = nullptr;
	switch (c) {
	case uint8_t('i'):
	{
		int8_type t;
		is.read(reinterpret_cast<char*>(&t.data_), 1);
		pointer = &t;
		break;
	}
	case uint8_t('U'):
	{
		uint8_type t;
		is.read(reinterpret_cast<char*>(&t.data_), 1);
		pointer = &t;
		break;
	}
	case uint8_t('I'):
	{
		int16_type t;
		is.read(reinterpret_cast<char*>(&t.data_), 2);
		le2be(t.data_, 2);
		pointer = &t;
		break;
	}
	case uint8_t('l'):
	{
		int32_type t;
		is.read(reinterpret_cast<char*>(&t.data_), 4);
		pointer = &t;
		break;
	}
	case uint8_t('d'):
	{
		float32_type t;
		is.read(reinterpret_cast<char*>(&t.data_), 4);
		pointer = &t;
		break;
	}
	case uint8_t('D'):
	{
		float64_type t;
		is.read(reinterpret_cast<char*>(&t.data_), 8);
		pointer = &t;
		break;
	}
	case uint8_t('C'):
	{
		char_type t;
		is.read(reinterpret_cast<char*>(&t.data_), 1);
		pointer = &t;
		break;
	}
	case uint8_t('S'):
	{
		string_type t;
		elem* el = read_val(is);
		t.len_ = el;
		is.read(reinterpret_cast<char*>(&t.data_[0]), t.len_->data_);
		pointer = &t;
		break;
	}
	case uint8_t('{'):
	{
		object_type ob;
		while (is.peek() != '}') {
			std::pair<std::string, elem*> p = read_couple(is);
			ob.map_[p.first] = p.second;
		}
		is.get();
		pointer = &ob;
		break;
	}
	case uint8_t('['):
	{
		array_type v;
		while (is.peek() != ']') {
			char tmp;
			tmp = is.get();
			if (tmp == '$') {
				is.read(reinterpret_cast<char*>(&v.dtype_), 1);
				tmp = is.get();
			}
			if (tmp == '#') {
				elem* el = read_val(is);
				v.len_ = el;
				for (size_t i = 0; i < v.len_->data_; ++i) {
					v.arrdata_.push_back(read_val(is, v.dtype_, false)->data_);
				}
			}
		}
		is.get();
		pointer = &v;
		break;
	}
	}
	return pointer;
}

std::pair<std::string, elem*> read_couple(std::ifstream& is) {
	is.get();
	uint8_t c;
	std::string s;
	read_key(is, s);
	elem* el = read_val(is);
	std::pair<std::string, elem*> p{ s, el };
	return p;
}

void read_canvas(std::ifstream& is, canvas& c) {
	is.get();
	std::pair<std::string, elem*> p = read_couple(is);
	object_type* ob = dynamic_cast<object_type*>(p.second);
	c.w_ = ob->map_["width"]->data_;
	c.h_ = ob->map_["height"]->data_;
	c.background_.R_ = dynamic_cast<array_type*>(ob->map_["background"])->arrdata_[0];
	c.background_.G_ = dynamic_cast<array_type*>(ob->map_["background"])->arrdata_[1];
	c.background_.B_ = dynamic_cast<array_type*>(ob->map_["background"])->arrdata_[2];
}

void read_elements(std::ifstream& is) {
	image<rgb> img;

	is.get();
	std::pair<std::string, elem*> p = read_couple(is);
	object_type* ob = dynamic_cast<object_type*>(p.second);
	object_type* im = dynamic_cast<object_type*>(ob->map_["image"]);
	int x_off = im->map_["x"]->data_;
	int y_off = im->map_["y"]->data_;
	int w = im->map_["width"]->data_;
	int h = im->map_["height"]->data_;
	img.resize(w, h);
	for (size_t i = 0; i < img.data_size(); ++i) {
		img.data()[i] = dynamic_cast<array_type*>(im->map_["data"])->arrdata_[i];
	}
}

int convert(const std::string& ifile, const std::string& ofile) {

	// Un chiarimento su UBJ: tutte le chiavi delle chiavi/valore json devono necessariamente essere stringhe
	// quindi vengono rappresentate come stringhe ma omettendo obbligatoriamente la [S] (per questo si specifica la
	// length dopo la [i].
	// 
	// Dal file UBJ devo estrarre le informazioni e creare il canvas

	std::ifstream is(ifile, std::ios::binary);

	canvas c;

	read_canvas(is, c);

	read_elements(is);

	//std::map<std::string, std::unique_ptr<elem>> map;


	unsigned w = 100; // TODO : modificare
	unsigned h = 100; // TODO : modificare

	image<vec3b> img(w, h);

	// Per accedere ai pixel di img posso usare img(x,y) oppure img.begin() e img.end()

	// Dal file UBJ devo estrarre le informazioni sulle immagini da incollare su img 

	// Output in formato PPM
	if (!writeP6(ofile, img))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		std::cout << "Syntax Error." << std::endl;
		return EXIT_FAILURE;
	}
	
	std::string ifile = argv[1];
	std::string ofile = argv[2];

	return convert(ifile, ofile);
}