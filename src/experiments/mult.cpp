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

int mult(int party_id, uint32_t x, uint32_t y) {
    int         port         = comm::kDefaultPort;
    std::string host_address = comm::kDefaultAddress;

    uint32_t                                     bitsize = 32;
    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);

    comm::CommInfo               comm_info(party_id, port, host_address);
    tools::secret_sharing::Party party(comm_info);

    party.StartCommunication();

    tools::secret_sharing::ShareHandler  sh;
    tools::secret_sharing::BeaverTriplet bt;

    if (party_id == 0) {
        tools::secret_sharing::bts_t bt_vec_0(1);
        sh.LoadBTShare("file_path", bt_vec_0);
        bt = bt_vec_0[0];
    } else {
        tools::secret_sharing::bts_t bt_vec_1(1);
        sh.LoadBTShare("file_path", bt_vec_1);
        bt = bt_vec_1[0];
    }

    uint32_t z = ss.Mult(party, bt, x, y);

    party.EndCommunication();

    return z;
}
