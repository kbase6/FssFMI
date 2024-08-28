/**
 * @file comm_configure.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Communication configure class.
 */

#ifndef INTERNAL_COMM_CONFIGURE_H_
#define INTERNAL_COMM_CONFIGURE_H_

#include <arpa/inet.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace comm {
namespace internal {

/**
 * @brief Sends data through a socket file descriptor.
 *
 * Sends the provided 'data' of size 'data_size' through the socket file descriptor 'fd'.
 * It ensures that all data is sent completely before returning.
 *
 * @param fd The file descriptor representing the socket connection.
 * @param data Pointer to the data to be sent.
 * @param data_size The size of the data to be sent.
 * @return True if the data is sent successfully; otherwise, false.
 */
inline bool SendData(int fd, const char *data, size_t data_size) {
    ssize_t total_sent_bytes = 0;
    while (total_sent_bytes < static_cast<ssize_t>(data_size)) {
        ssize_t sent_bytes = send(fd, data, data_size, 0);
        if (sent_bytes <= 0) {
            std::perror("send data");
            return false;
        }
        total_sent_bytes += sent_bytes;
    }
    return true;
}

/**
 * @brief Receives data through a socket file descriptor.
 *
 * Receives data from the socket file descriptor 'fd' and stores it in the 'buffer'.
 * It ensures that all expected data is received completely before returning.
 *
 * @param fd The file descriptor representing the socket connection.
 * @param buffer Pointer to the buffer where received data will be stored.
 * @param buffer_size The size of the buffer to store the received data.
 * @return True if the data is received successfully; otherwise, false.
 */
inline bool RecvData(int fd, char *buffer, size_t buffer_size) {
    ssize_t total_received_bytes = 0;
    while (total_received_bytes < static_cast<ssize_t>(buffer_size)) {
        ssize_t received_bytes = recv(fd, buffer, buffer_size, 0);
        if (received_bytes <= 0) {
            std::perror("receive data");
            return false;
        }
        total_received_bytes += received_bytes;
    }
    return true;
}

}    // namespace internal
}    // namespace comm

#endif    // INTERNAL_COMM_CONFIGURE_H_
