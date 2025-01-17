#ifndef SECURE_MULT
#define SECURE_MULT

#include <cstdint>
#include "../tools/secret_sharing.hpp"

uint32_t mult(tools::secret_sharing::Party &party, uint32_t x, uint32_t y, const uint32_t bitsize);

#endif    // SECURE_MULT
