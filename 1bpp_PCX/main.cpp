#include "pcx.h"
#include "pgm.h"
#define esercizio2

#ifdef esercizio1

int main(int argc, char** argv) {
	mat<uint8_t> im;
	std::string ifile = argv[1];
	if (!load_pcx(ifile, im)) return EXIT_FAILURE;
	if (!save_pgm("out1bpp.pgm", im)) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

#endif //esercizio1


#ifdef esercizio2

int main(int argc, char** argv) {
	mat<vec3b> im;
	std::string ifile = argv[1];
	if (!load_pcx(ifile, im)) return EXIT_FAILURE;


	mat<uint8_t> r, g, b;
	r.resize(im.rows(), im.rows()); g.resize(im.rows(), im.rows()); b.resize(im.rows(), im.rows());
	for (int i = 0; i < im.size(); i++) {
		vec3b d = im[i];
		r[i] = d[0]; g[i] = d[1]; b[i] = d[2];
	}

	save_pgm("outR.pgm", r);
	save_pgm("outG.pgm", g);
	save_pgm("outB.pgm", b);



	return EXIT_SUCCESS;
}

#endif //esercizio2