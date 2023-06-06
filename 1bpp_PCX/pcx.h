#ifndef PCX_H
#define PCX_H

#include "mat.h"
#include <string>

bool load_pcx(const std::string& filename, mat<uint8_t>& img);

#endif // PCX_H