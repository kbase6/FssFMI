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
#include "select.hpp"

// Test caculation: x = 10, y = 20 return larger integer
int main() {
    int         port         = comm::kDefaultPort;
    std::string host_address = comm::kDefaultAddress;
    int         party_id     = 1;

    comm::CommInfo               comm_info(party_id, port, host_address);
    tools::secret_sharing::Party party(comm_info);

    uint32_t bitsize = 32;

    uint32_t x = 4;
    uint32_t y = 6;
    uint32_t b = fss::Compare(party, x, y, bitsize);

    uint32_t res = select(party, b, x, y, bitsize);
    return res;
}
