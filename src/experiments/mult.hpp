#ifndef SECURE_MULT
#define SECURE_MULT

#include <cstdint>
#include "../tools/secret_sharing.hpp"

// int mult(int party_id, uint32_t x, uint32_t y);
int mult(tools::secret_sharing::Party &party, uint32_t x, uint32_t y);

#endif    // SECURE_MULT
