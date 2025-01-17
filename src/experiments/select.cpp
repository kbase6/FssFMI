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
#include "mult.hpp"
#include "select.hpp"

uint32_t select(tools::secret_sharing::Party &party, uint32_t b, uint32_t x, uint32_t y, const uint32_t bitsize) {
    /*
    if b is 1, return x; otherwise, return y.
    */
    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);

    party.StartCommunication();

    tools::secret_sharing::ShareHandler  sh;
    tools::secret_sharing::BeaverTriplet bt;

    if (party.GetId() == 0) {
        tools::secret_sharing::bts_t bt_vec_0(1);
        sh.LoadBTShare("/home/matsuda/FssFMI/data/test/ss/bt_0", bt_vec_0);
        bt = bt_vec_0[0];
    } else {
        tools::secret_sharing::bts_t bt_vec_1(1);
        sh.LoadBTShare("/home/matsuda/FssFMI/data/test/ss/bt_1", bt_vec_1);
        bt = bt_vec_1[0];
    }

    // Compute (x-y) mod 2^bitsize
    uint32_t delta = utils::Mod(x - y, 1u << bitsize);

    // Perform secure multiplication
    uint32_t z = mult(party, b, delta);

    uint32_t res = add(y, z);

    party.EndCommunication();

    return res;
}
