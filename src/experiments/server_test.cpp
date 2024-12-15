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

// Test calculation: (4 + 6) * 3
// Shares 4: 1, 3
//        6: 4, 2
//        3: 1, 2
int main() {
    int      party_id = 0;
    uint32_t x        = 1;
    uint32_t y        = 4;
    uint32_t w        = 1;

    std::cout << "x: " << x << std::endl;
    std::cout << "y: " << y << std::endl;
    std::cout << "w: " << w << std::endl;

    uint32_t z = add(x, y);

    std::cout << "add result: " << z << std::endl;

    uint32_t res = mult(party_id, z, w);

    std::cout << "mult result: " << res << std::endl;

    return res;
}
