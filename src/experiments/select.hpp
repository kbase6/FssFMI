#ifndef SECURE_SELECT
#define SECURE_SELECT

#include "../tools/secret_sharing.hpp"
#include <cstdint>

uint32_t select(tools::secret_sharing::Party &party, uint32_t b, uint32_t x, uint32_t y, const uint32_t bitsize = 32);

#endif    // SECURE_SELECT
