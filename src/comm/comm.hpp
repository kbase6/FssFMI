/**
 * @file comm.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Communication class.
 */

#ifndef COMM_H_
#define COMM_H_

#include <string>
#include <cstdint>

namespace comm {

constexpr int  kDefaultPort      = 55555;          // Default port number for communication
constexpr char kDefaultAddress[] = "127.0.0.1";    // Default IP address for communication

/**
 * @brief Structure to store communication information.
 *
 * Structure representing communication information containing:
 */
struct CommInfo {
    int         party_id;     /**< Identifier for the party. */
    int         port_number;  /**< Port number for communication. */
    std::string host_address; /**< Host address or IP for connection. */

    /**
     * @brief Constructor to initialize CommInfo.
     *
     * Initializes CommInfo with the provided 'id', 'port', and 'address'.
     *
     * @param id The identifier for the party.
     * @param port The port number for communication.
     * @param address The host address or IP for connection.
     */
    CommInfo(int id, int port, std::string address)
        : party_id(id), port_number(port), host_address(address) {
    }
};

namespace test {

void Test_Comm(const CommInfo &comm_info, const uint32_t mode, bool debug);

}    // namespace test

}    // namespace comm

#endif    // COMM_H_
