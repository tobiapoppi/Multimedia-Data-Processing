#ifndef PCX_H
#define PCX_H
#define esercizio1


#ifdef esercizio1

#include "mat.h"
#include <string>

bool load_pcx(const std::string& filename, mat<uint8_t>& img);

#endif //esercizio 1


#ifdef esercizio2

#include "mat.h"
#include <string>
#include "types.h"

bool load_pcx(const std::string& filename, mat<vec3b>& img);

#endif //esercizio 2

#ifdef esercizio3

#include "mat.h"
#include <string>
#include "types.h"

bool load_pcx(const std::string& filename, mat<vec3b>& img);

#endif //esercizio 3

#endif // PCX_H