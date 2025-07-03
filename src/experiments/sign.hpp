#ifndef SECURE_SIGN
#define SECURE_SIGN

#include "../fss-base/dcf/distributed_comparison_function.hpp"
#include "../tools/secret_sharing.hpp"
#include <cstdint>

uint32_t sign(tools::secret_sharing::Party &party, uint32_t k, uint32_t y);

#endif    // SECURE_SIGN
