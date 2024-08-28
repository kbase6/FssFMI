/**
 * @file client.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Client class.
 */

#ifndef COMM_CLIENT_H_
#define COMM_CLIENT_H_

#include <array>
#include <vector>

#include "internal/comm_configure.hpp"

namespace comm {

/**
 * @class Client
 * @brief A class representing a communication client for handling network interactions.
 */
class Client {
public:
    /**
     * @brief Constructs a Client object with specified host address, port, and debug mode.
     *
     * Constructs a Client object with the provided 'host_address', 'port', and 'debug' mode settings.
     *
     * @param host_address The host address or IP to connect to.
     * @param port The port number to establish the connection.
     * @param debug If true, enables debug mode; if false, debug mode is disabled.
     */
    Client(std::string host_address, int port, bool debug);

    /**
     * @brief Destroys the Client object.
     *
     * Closes the socket connection associated with the Client object upon destruction.
     */
    ~Client();

    /**
     * @brief Sets up the client socket.
     *
     * Creates a socket for the client to establish a connection.
     */
    void Setup();

    /**
     * @brief Closes the client socket.
     *
     * Closes the socket connection associated with the Client object.
     * This method should be called to properly close the socket and release resources.
     */
    void CloseSocket();

    /**
     * @brief Initiates connection to the server.
     *
     * Establishes a connection to the server using the created socket.
     *
     * This method should be called to connect the client to the server.
     */
    void Start();

    /**
     * @brief Sends a uint32_t value to the connected server.
     *
     * Sends the provided 'value' as a uint32_t data to the connected server through the socket.
     *
     * @param value The uint32_t value to be sent to the server.
     */
    void SendValue(uint32_t value);

    /**
     * @brief Receives a uint32_t value from the connected server.
     *
     * Receives a uint32_t data from the connected server through the socket
     * and stores it in the provided 'value'.
     *
     * @param value Reference to a uint32_t variable to store the received data.
     */
    void RecvValue(uint32_t &value);

    /**
     * @brief Sends a std::vector<uint32_t> to the connected server.
     *
     * Sends the provided 'vector' of type std::vector<uint32_t> to the connected server
     * through the socket.
     *
     * @param vector Reference to an std::vector<uint32_t> to be sent to the server.
     *
     * @warning Significantly worse performance than value and array
     */
    void SendVector(std::vector<uint32_t> &vector);

    /**
     * @brief Receives an std::vector<uint32_t> from the connected client.
     *
     * Receives an std::vector<uint32_t> from the connected client through the socket
     * and stores it in the provided 'vector'.
     *
     * @param vector Reference to an std::vector<uint32_t> to store the received data.
     *
     * @warning Significantly worse performance than value and array
     */
    void RecvVector(std::vector<uint32_t> &vector);

    /**
     * @brief Sends an std::array<uint32_t, 2> to the connected client.
     *
     * Sends the provided 'array' of type std::array<uint32_t, 2> to the connected client
     * through the socket.
     *
     * @param array Reference to an std::array<uint32_t, 2> to be sent to the client.
     */
    void SendArray(std::array<uint32_t, 2> &array);

    /**
     * @brief Receives an std::array<uint32_t, 2> from the connected client.
     *
     * Receives an std::array<uint32_t, 2> from the connected client through the socket
     * and stores it in the provided 'array'.
     *
     * @param array Reference to an std::array<uint32_t, 2> to store the received data.
     */
    void RecvArray(std::array<uint32_t, 2> &array);

    /**
     * @brief Sends an std::array<uint32_t, 4> to the connected client.
     *
     * Sends the provided 'array' of type std::array<uint32_t, 4> to the connected client
     * through the socket.
     *
     * @param array Reference to an std::array<uint32_t, 4> to be sent to the client.
     */
    void SendArray(std::array<uint32_t, 4> &array);

    /**
     * @brief Receives an std::array<uint32_t, 4> from the connected client.
     *
     * Receives an std::array<uint32_t, 4> from the connected client through the socket
     * and stores it in the provided 'array'.
     *
     * @param array Reference to an std::array<uint32_t, 4> to store the received data.
     */
    void RecvArray(std::array<uint32_t, 4> &array);

    /**
     * @brief Retrieves the host address used by the client.
     *
     * Returns the host address or IP used by the client for connection.
     *
     * @return A string containing the host address used by the client.
     */
    std::string GetHostAddress();

    /**
     * @brief Retrieves the port number used by the client.
     *
     * Returns the port number used by the client for communication.
     *
     * @return An integer representing the port number used by the client.
     */
    int GetPortNumber();

    /**
     * @brief Retrieves the total number of bytes sent to the server.
     *
     * Returns the total number of bytes sent to the server through the socket.
     *
     * @return An unsigned integer representing the total number of bytes sent to the server.
     */
    uint32_t GetTotalBytesSent() const;

    /**
     * @brief Clears the total number of bytes sent to the server.
     *
     * Resets the total number of bytes sent to the server to zero.
     */
    void ClearTotalBytesSent();

private:
    std::string host_address_;     /**< Host address of the server */
    int         port_;             /**< Port number used for the connection */
    int         client_fd_;        /**< File descriptor for the client socket */
    bool        debug_;            /**< Flag indicating debug mode. */
    uint32_t    total_bytes_sent_; /**< Total number of bytes sent to the server */
};

}    // namespace comm

#endif    // COMM_CLIENT_H_
