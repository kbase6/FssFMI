#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "../comm/comm.hpp"
#include "../tools/random_number_generator.hpp"
#include "../tools/secret_sharing.hpp"
#include "../tools/tools.hpp"
#include "../utils/logger.hpp"
#include "../utils/utils.hpp"

int mult(int arg, char *argv[]) {
    int port = comm::kDefaultPort;
    std::string host_address = comm::kDefaultAddress;

    uint32_t bitsize = 32;
    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);
    
    int party_id = std::stoi(argv[1]);
    uint32_t x = std::stoi(argv[2]);
    uint32_t y = std::stoi(argv[3]);

    comm::CommInfo comm_info(party_id, port, host_address);
    tools::secret_sharing::Party party(comm_info);

    
}
