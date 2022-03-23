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

	std::vector<uint8_t> v{ std::istream_iterator<uint8_t>(is),
		std::istream_iterator<uint8_t>() };

	std::ofstream os(argv[2], std::ios::binary);
	std::map<uint8_t, int> m;

	for (const auto& t : v) {
		const int counter = std::count(v.cbegin(), v.cend(), t);
		m.insert(std::pair<uint8_t, int>(t, counter));
	}

	for (const auto& t : m) {
		os << std::hex << std::get<0>(t);
		os << '\t' << std::get<1>(t) << '\n';
	}
}