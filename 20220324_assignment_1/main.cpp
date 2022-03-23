#include<fstream>
#include<vector>
#include<algorithm>
#include<map>

int main(int argc, char* argv[]) {

	if (argc != 3)
		return EXIT_FAILURE;

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}

	std::vector<uint8_t> v{std::istream_iterator<uint8_t>(is),
		std::istream_iterator<uint8_t>()};

	std::ofstream os(argv[2]);
	std::map<uint8_t, int> m;

	for (const uint8_t t : v) {
		const int counter = std::count(v.cbegin(), v.cend(), t);
		m.insert(std::pair<uint8_t, int>(t, counter));
	}

	for (const std::pair<uint8_t, int> t : m) {
		os << (uint8_t)t.first << '\t' << t.second << '\n';
	}
}