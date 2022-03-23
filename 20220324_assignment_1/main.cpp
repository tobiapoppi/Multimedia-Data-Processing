#include<fstream>
#include<vector>

int main(int argc, char* argv[]) {

	if (argc != 3)
		return EXIT_FAILURE;

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}

	std::vector<uint8_t> v;

}