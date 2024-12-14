#include <iostream>
#include "./comm/server.hpp"
#include "./tools/secret_sharing.hpp"

int main() {
    uint32_t x = 10;

    int party_id = 0;
    int port = comm:kDefaultPort;
    std::string host_address = comm::kDefaultAddress;

    comm::CommInfo comm_info(party_id, port, host_address);
    tools::secret_sharing::Party party(comm_info);

    bool debug = false;
    Server p0(comm_info.port_number, debug);

    p0.Setup();
    p0.Start();

    std::cout << "Sending value: " << x;

    p0.SendValue(x);
    // p0.RecvValue(x);
    return;
}
