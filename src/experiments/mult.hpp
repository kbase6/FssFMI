#ifndef SECURE_MULT
#define SECURE_MULT

#include "../tools/secret_sharing.hpp"
#include <cstdint>

uint32_t mult(tools::secret_sharing::Party &party, uint32_t x, uint32_t y, const uint32_t bitsize = 32);

#endif    // SECURE_MULT
