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

uint32_t mult(tools::secret_sharing::Party &party, uint32_t x, uint32_t y, const uint32_t bitsize) {

    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);

    party.StartCommunication();

    tools::secret_sharing::ShareHandler  sh;
    tools::secret_sharing::BeaverTriplet bt;

    if (party.GetId() == 0) {
        tools::secret_sharing::bts_t bt_vec_0(1);
        // TODO: Change the path of the Beaver Triple dynamically
        sh.LoadBTShare("/home/matsuda/FssFMI/data/test/ss/bt_0", bt_vec_0);
        bt = bt_vec_0[0];
    } else {
        tools::secret_sharing::bts_t bt_vec_1(1);
        // TODO: Change the path of the Beaver Triple dynamically
        sh.LoadBTShare("/home/matsuda/FssFMI/data/test/ss/bt_1", bt_vec_1);
        bt = bt_vec_1[0];
    }
    // std::cout << "Beaver triplet: " << bt.a << ", " << bt.b << ", " << bt.c << std::endl;
    uint32_t res = ss.Mult(party, bt, x, y);

    party.EndCommunication();

    return res;
}
