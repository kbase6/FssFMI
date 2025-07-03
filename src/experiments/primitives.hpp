#ifndef PRIMITIVES_H_
#define PRIMITIVES_H_

#include "../comm/comm.hpp"
#include "../tools/secret_sharing.hpp"

namespace primitives {

uint32_t Add(uint32_t x, uint32_t y, const uint32_t bitsize = 32);
uint32_t Mult(tools::secret_sharing::Party &party, uint32_t x, uint32_t y, const uint32_t bitsize = 32);
uint32_t Select(tools::secret_sharing::Party &party, uint32_t b, uint32_t x, uint32_t y, const uint32_t bitsize = 32);

}    // namespace primitives

#endif
