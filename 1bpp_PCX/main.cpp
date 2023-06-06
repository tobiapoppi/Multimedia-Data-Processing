#include "pcx.h"
#include "pgm.h"

int main(int argc, char** argv) {
	mat<uint8_t> im;
	std::string ifile = argv[1];
	if (!load_pcx(ifile, im)) return EXIT_FAILURE;
	if (!save_pgm("out1bpp.pgm", im)) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}