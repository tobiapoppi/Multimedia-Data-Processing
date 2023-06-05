#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <functional>
#include <exception>

#include "ppm.h"
#include "bitstreams.h"
#include "image_operations.h"

struct rgb {
	uint8_t R_, G_, B_;
};

struct canvas {
	int w_, h_, x_off, y_off;
	rgb background_;
};

struct img_place {
	int x_anch;
	int y_anch;
	image<rgb> img;
};

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

bool find_string(ifstream& is, string name) {
    int len = name.size();
    int l;
    uint8_t b;
    string s;
    while (is) {
        b = is.get();
        while (b != 'i') { b = is.get(); if (!is) return false; }

        l = is.get();
        if (l != len) continue;

        s = "";
        for (int i = 0; i < len; i++) { s += is.get(); if (!is) return false; }

        if (s == name) return true;
    }
    return true;
}

int read_int(ifstream& is) {
    uint8_t type = is.get();
    uint8_t val;
    int out = 0;
    if (type == 'i' or type == 'U') {
        is.read((char*)&val, 1);
        out = val;
    }
    else if (type == 'I') {
        is.read((char*)&val, 1);
        out = val;
        is.read((char*)&val, 1);
        out = out << 8 | val;
    }
    else if (type == 'l') {
        for (int i = 0; i < 4; i++) {
            is.read((char*)&val, 1);
            out = out << 8 | val;
        }
    }
    return out;
}

bool read_canvas(ifstream& is, canvas& can) {
    uint8_t b;
    uint8_t len;
    string s;
    // Read canvas data
    if (!find_string(is, "canvas")) return false;
    find_string(is, "width");
    can.w_ = read_int(is);
    find_string(is, "height");
    can.h_ = read_int(is);
    find_string(is, "background");
    is.ignore(6);

    is.read((char*)&can.background_.R_, 1);
    is.read((char*)&can.background_.G_, 1);
    is.read((char*)&can.background_.b_, 1);

    return true;
}

bool read_elements(ifstream& is) {
    find_string(is, "elements");
    is.ignore();
    uint8_t b;
    int len;
    string s;
    while (is) {
        // Leggi la stringa di inizio
        b = 0; len = 0; s = "";
        is.read((char*)&b, 1);
        if (b == '}') continue;
        is.seekg(-1, ios::cur);

        len = read_int(is);
        if (!is) break;
        for (int i = 0; i < len; i++) {
            is.read((char*)&b, 1);
            s += b;
        }

        cout << s << " : ";

        // Leggo le diverse key per ogni element
        while (true) {
            is.read((char*)&b, 1);
            if (b == '}') break;
            while (b != 'i') { if (b == '}') break; is.read((char*)&b, 1); }
            is.seekg(-1, ios::cur);
            len = read_int(is);
            s = "";
            for (int i = 0; i < len; i++) {
                is.read((char*)&b, 1);
                s += b;
            }
            is.read((char*)&b, 1);
            if (b == '[') {
                is.ignore(3);
                len = read_int(is);
                is.ignore(len);
            }
            cout << s << ",";
        }
        cout << endl;

    }
    return true;

}

bool read_image(ifstream& is, img_place& imp) {
    int x, y, w, h;
    if (!find_string(is, "image")) return false;
    find_string(is, "x");
    x = read_int(is);
    find_string(is, "y");
    y = read_int(is);
    find_string(is, "width");
    w = read_int(is);
    find_string(is, "height");
    h = read_int(is);
    imp.x_anch = x;
    imp.y_anch = y;
    imp.img.resize(w, h);
    find_string(is, "data");
    is.ignore(4);
    read_int(is);
    is.read(imp.img.data(), imp.img.data_size());

    return true;
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


	unsigned w = c.w_; // TODO : modificare
	unsigned h = c.h_; // TODO : modificare

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