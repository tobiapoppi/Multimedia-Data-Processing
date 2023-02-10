#ifndef PGM_H
#define PGM_H

#include <string>
#include "matrix.h"

enum class pgm_mode { plain = 2, binary = 5 };

bool write(const std::string& filename, const matrix<uint8_t>& im, pgm_mode mode);

#endif

