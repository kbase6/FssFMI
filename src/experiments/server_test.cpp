#include <functional>
#include <getopt.h>
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
#include "add.hpp"
#include "fssgate.hpp"
#include "mult.hpp"

// Test calculation: (5 + 8) * 3
// Shares 5: 2, 3
//        8: 7, 1
//        3: 1, 2
int main() {
    int      party_id = 0;
    uint32_t x        = 2;
    uint32_t y        = 7;
    uint32_t w        = 1;

    uint32_t z = add(x, y);

    uint32_t res = mult(party_id, z, w);

    return res;
}
