#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../comm/comm.hpp"
#include "../tools/random_number_generator.hpp"
#include "../tools/secret_sharing.hpp"
#include "../tools/tools.hpp"
#include "../utils/logger.hpp"
#include "../utils/utils.hpp"

uint32_t add(uint32_t x, uint32_t y, const uint32_t bitsize) {

    uint32_t z       = utils::Mod(x + y, bitsize);

    return z;
}
