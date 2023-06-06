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

	return EXIT_SUCCESS;
}

#endif //esercizio2