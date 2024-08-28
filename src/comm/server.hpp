/**
 * @file server.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Server class.
 */

#ifndef COMM_SERVER_H_
#define COMM_SERVER_H_

#include "internal/comm_configure.hpp"

#include <array>
#include <vector>

namespace comm {

/**
 * @brief A class representing a server.
 *
 * This class provides functionality to set up and manage a server.
 */
class Server {
public:
    /**
     * @brief Constructs a Server object with a specified port and debug mode.
     *
     * Constructs a Server object with the provided 'port' and 'debug' mode settings.
     *
     * @param port The port number on which the server will listen.
     * @param debug If true, enables debug mode; if false, debug mode is disabled.
     */
    Server(const int port, const bool debug);

    /**
     * @brief Destroys the Server object.
     *
     * Closes the socket connection associated with the Server object upon destruction.
     */
    ~Server();

    /**
     * @brief Sets up the server socket and prepares it for incoming connections.
     *
     * Initializes and configures the server socket, binds it to the specified port,
     * and sets it to listen for incoming connections.
     */
    void Setup();

    /**
     * @brief Closes the server and client sockets.
     *
     * Closes the server socket and the client socket associated with the Server object.
     * This method should be called to properly close the sockets and release resources.
     */
    void CloseSocket();

    /**
     * @brief Starts the server and waits for incoming client connections.
     *
     * Accepts incoming client connections on the server socket and establishes a connection.
     * It waits for a client to connect and sets up a connection with the client.
     * This method should be called to initiate the server's operation.
     */
    void Start();

    /**
     * @brief Sends a uint32_t value to the connected client.
     *
     * Sends the provided 'value' as a uint32_t data to the connected client through the socket.
     *
     * @param value The uint32_t value to be sent to the client.
     */
    void SendValue(uint32_t value);

    /**
     * @brief Receives a uint32_t value from the connected client.
     *
     * Receives a uint32_t data from the connected client through the socket and stores it in 'value'.
     *
     * @param value Reference to a uint32_t variable to store the received data.
     */
    void RecvValue(uint32_t &value);

    /**
     * @brief Sends an std::vector<uint32_t> to the connected client.
     *
     * Sends the provided 'vector' of type std::vector<uint32_t> to the connected client
     * through the socket.
     *
     * @param vector Reference to an std::vector<uint32_t> to be sent to the client.
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
     * @brief Retrieves the port number used by the server.
     *
     * Returns the port number used by the server for communication.
     *
     * @return An integer representing the port number used by the server.
     */
    int GetPortNumber() const;

    /**
     * @brief Retrieves the total number of bytes sent to the client.
     *
     * Returns the total number of bytes sent to the client through the socket.
     *
     * @return An unsigned integer representing the total number of bytes sent to the client.
     */
    uint32_t GetTotalBytesSent() const;

    /**
     * @brief Clears the total number of bytes sent to the client.
     *
     * Resets the total number of bytes sent to the client to zero.
     */
    void ClearTotalBytesSent();

private:
    int      port_;             /**< The port number used for the server. */
    int      server_fd_;        /**< File descriptor for the server socket. */
    int      client_fd_;        /**< File descriptor for the client socket. */
    bool     debug_;            /**< Flag indicating debug mode. */
    uint32_t total_bytes_sent_; /**< Total number of bytes sent to the client. */
};

}    // namespace comm

#endif    // COMM_SERVER_H_
