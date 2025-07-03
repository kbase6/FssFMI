#include "primitives.hpp"

#include "../utils/file_io.hpp"
#include "../utils/logger.hpp"
#include "../utils/utils.hpp"

namespace primitives {

uint32_t Add(uint32_t x, uint32_t y, const uint32_t bitsize) {

    uint32_t result = utils::Mod(x + y, bitsize);

    return result;
}

uint32_t Mult(tools::secret_sharing::Party &party, uint32_t x, uint32_t y, const uint32_t bitsize) {

    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);
    tools::secret_sharing::ShareHandler          sh;
    tools::secret_sharing::BeaverTriplet         bt;

    party.StartCommunication();

    if (party.GetId() == 0) {
        tools::secret_sharing::bts_t bt_vec_0(1);
        sh.LoadBTShare("/home/matsuda/FssFMI/data/test/ss/bt_0", bt_vec_0);    // TODO: Change the path of the Beaver Triple dynamically
        bt = bt_vec_0[0];
    } else {
        tools::secret_sharing::bts_t bt_vec_1(1);
        sh.LoadBTShare("/home/matsuda/FssFMI/data/test/ss/bt_1", bt_vec_1);    // TODO: Change the path of the Beaver Triple dynamically
        bt = bt_vec_1[0];
    }

    uint32_t result = ss.Mult(party, bt, x, y);

    return result;
}

uint32_t Select(tools::secret_sharing::Party &party, uint32_t b, uint32_t x, uint32_t y, const uint32_t bitsize) {
    /*
    if b is 1, return x; otherwise, return y.
    Computes b(x-y) + y
    */
    tools::secret_sharing::ShareHandler  sh;
    tools::secret_sharing::BeaverTriplet bt;

    party.StartCommunication();

    if (party.GetId() == 0) {
        tools::secret_sharing::bts_t bt_vec_0(1);
        sh.LoadBTShare("/home/matsuda/FssFMI/data/test/ss/bt_0", bt_vec_0);    // TODO: Change the path following other codes
        bt = bt_vec_0[0];
    } else {
        tools::secret_sharing::bts_t bt_vec_1(1);
        sh.LoadBTShare("/home/matsuda/FssFMI/data/test/ss/bt_1", bt_vec_1);    // TODO: Change the path following other codes
        bt = bt_vec_1[0];
    }

    // Compute (x-y) mod 2^bitsize
    uint32_t delta  = utils::Mod(x - y, bitsize);
    uint32_t z      = Mult(party, b, delta, bitsize);
    uint32_t result = Add(y, z, bitsize);

    return result;
}

}    // namespace primitives
