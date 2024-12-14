#include "./comm/client.hpp"
#include "./tools/secret_sharing.hpp"
#include <iostream>

int main()
{
    int port = comm::kDefaultPort;
    std::string host_address = comm::kDefaultAddress;
    int party_id = 1;

    comm::CommInfo comm_info(party_id, port, host_address);
    tools::secret_sharing::Party party(comm_info);

    bool debug = false;
    comm::Client p1(comm_info.host_address, comm_info.port_number, debug);

    p1.Setup();
    p1.Start();

    uint32_t r_x;
    p1.RecvValue(r_x);
    std::cout << r_x;
}
