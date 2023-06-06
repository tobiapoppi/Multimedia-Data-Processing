#ifndef PCX_H
#define PCX_H

#ifdef esercizio1

#include "mat.h"
#include <string>

bool load_pcx(const std::string& filename, mat<uint8_t>& img);

#endif //esercizio 1

#define esercizio2

#ifdef esercizio2

#include "mat.h"
#include <string>
#include "types.h"

bool load_pcx(const std::string& filename, mat<vec3b>& img);

#endif //esercizio 2

#endif // PCX_H