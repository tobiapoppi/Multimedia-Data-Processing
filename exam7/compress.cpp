#include <vector>
#include "mat.h"
#include "ppm.h"

void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	uint8_t curr;
	uint8_t next;
	uint8_t next_next;
	uint8_t copy_count;
	uint8_t run_count;
	std::vector<uint8_t> lit;
	size_t n = 0;
	bool end = false;

	while (true) {
		copy_count = 0;
		run_count = 0;
		lit.clear();
		curr = img.data()[n];

		//if I'm on the last position of input, I must end with a single literal
		if (n + 1 == img.size()) {
			encoded.push_back(0);
			encoded.push_back(curr);
			break;
		}

		next = img.data()[n + 1];

		//if the next position is the last one of input, I must end with a 2 literals of a 2-run.
		if (n + 2 == img.size()) {
			if (curr == next) {
				encoded.push_back(255);
				encoded.push_back(curr);
			}
			else {
				encoded.push_back(1);
				encoded.push_back(curr);
				encoded.push_back(next);
			}
			break;
		}

		if (curr != next) {
			//copy command
			lit.push_back(curr);
			++copy_count;
			next_next = img.data()[n + 2];

			while (curr != next && next != next_next && copy_count < 128) {
				curr = next;
				lit.push_back(curr);
				++n;
				++copy_count;
				next = next_next;

				if (n + 2 == img.size()) {
					//here I should increment copy_count to include next but I send directly copy_count instead of copy_count-1
					encoded.push_back(copy_count);
					for (size_t i = 0; i < lit.size(); ++i) {
						encoded.push_back(lit[i]);
					}
					encoded.push_back(next);
					end = true;
					break;
				}

				next_next = img.data()[n + 2];
			}
			if (end == true) {
				break;
			}
			encoded.push_back(copy_count - 1);
			for (size_t i = 0; i < lit.size(); ++i) {
				encoded.push_back(lit[i]);
			}
			++n;

		}
		else {
			//run command
			++run_count;
			while (curr == next && run_count <= 127) {
				++run_count;
				curr = next;
				++n;
				if (n + 1 == img.size()) {
					encoded.push_back(257 - run_count);
					encoded.push_back(curr);
					end = true;
					break;
				}
				next = img.data()[n + 1];
			}
			if (end == true) {
				break;
			}
			encoded.push_back(257 - run_count);
			encoded.push_back(curr);
			++n;
		}
	}
	encoded.push_back(128);
}