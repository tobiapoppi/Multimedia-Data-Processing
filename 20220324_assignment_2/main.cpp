#include<fstream>
#include<vector>
#include<algorithm>
#include<map>

int main(int argc, char* argv[]) {
	std::ifstream is;
	std::ofstream os;
	int val;

	if (argc != 3)
		exit(EXIT_FAILURE);

	is.open(argv[1]);
	if (!is)
		exit(EXIT_FAILURE);

	std::vector<int32_t> v{std::istream_iterator<int32_t>(is), 
		std::istream_iterator<int32_t>()};

	os.open(argv[2], std::ios::binary);
	if (!os)
		exit(EXIT_FAILURE);

	for (auto it = v.begin(); it != v.end(); it++) {
		os.write(reinterpret_cast<char*>(&(*it)), 4);
	}

	return 0;
}