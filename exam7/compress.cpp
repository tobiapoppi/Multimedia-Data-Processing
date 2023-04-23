#include <vector>
#include "mat.h"
#include "ppm.h"

void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	uint8_t curr;
	uint8_t prev;
	bool init = true;
	bool literalling = false;
	bool running = false;
	uint8_t cp_count = 1;
	std::vector<uint8_t> lit;

	for (size_t i = 0; i < img.rows(); ++i) {
		for (size_t j = 0; j < img.cols(); ++j) {
			curr = img(i, j);

			if (init == true) {
				prev = curr;
				init = false;
			}
			else {
				if (running) {
					if (curr == prev) {
						if (cp_count == 128) {
							encoded.push_back(129);
							encoded.push_back(curr);
							cp_count = 1;
							running = true;
							literalling = false;
						}
						else {
							++cp_count;
						}
					}
					else {
						encoded.push_back(257 - cp_count);
						encoded.push_back(prev);
						running = false;
						literalling = true;
						prev = curr;
						cp_count = 1;
					}
				}
				else if (literalling) {
					if (curr != prev) {
						if (lit.size() < 128) {
							lit.push_back(prev);
							prev = curr;
						}
						else {
							encoded.push_back(lit.size() - 1);
							for (size_t k = 0; k < lit.size(); ++k) {
								encoded.push_back(lit[k]);
							}
							lit.clear();
							literalling = true;
							prev = curr;
						}
					}
					else {
						encoded.push_back(lit.size() - 1);
						for (size_t k = 0; k < lit.size(); ++k) {
							encoded.push_back(lit[k]);
						}
						lit.clear();
						literalling = false;
						running = true;
						prev = curr;
						++cp_count;
					}
				}
				else {
					if (curr == prev) {
						running = true;
						literalling = false;
						++cp_count;
					}
					else {
						literalling = true;
						running = false;
						lit.push_back(prev);
						prev = curr;
					}
				}
			}
		}
	}
	encoded.push_back(128);
}