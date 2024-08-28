/**
 * @file client.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Client implementation.
 */

#include "client.hpp"

#include "../utils/logger.hpp"
#include "../utils/utils.hpp"

namespace comm {

Client::Client(std::string host_address, int port, bool debug)
    : host_address_(host_address), port_(port), debug_(debug) {
}

Client::~Client() {
    this->CloseSocket();
}

void Client::Setup() {
    // Create socket
    this->client_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    if (this->client_fd_ < 0) {
        utils::Logger::FatalLog(LOCATION, "Failed to create socket");
        exit(EXIT_FAILURE);
    }
    utils::Logger::TraceLog(LOCATION, "Created socket", this->debug_);
}

void Client::CloseSocket() {
    close(this->client_fd_);
}

void Client::Start() {
    // Setup socket address structure
    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(this->port_);
    server_address.sin_addr.s_addr = inet_addr(this->host_address_.c_str());

    // Connect to server
    int status = connect(this->client_fd_, (const sockaddr *)&server_address, sizeof(server_address));
    if (status < 0) {
        utils::Logger::FatalLog(LOCATION, "Failed to connect to the server");
        exit(EXIT_FAILURE);
    }
    utils::Logger::TraceLog(LOCATION, "Connected to the server", this->debug_);
}

void Client::SendValue(uint32_t value) {
    // Send data
    bool is_sent = internal::SendData(this->client_fd_, reinterpret_cast<const char *>(&value), sizeof(value));
    if (!is_sent) {
        utils::Logger::FatalLog(LOCATION, "Failed to send uint32_t data");
        this->CloseSocket();
        exit(EXIT_FAILURE);
    }
    this->total_bytes_sent_ += sizeof(value);
    utils::Logger::TraceLog(LOCATION, "Sent data: " + std::to_string(value), this->debug_);
}

void Client::RecvValue(uint32_t &value) {
    // Receive data
    bool is_received = internal::RecvData(this->client_fd_, reinterpret_cast<char *>(&value), sizeof(value));
    if (!is_received) {
        utils::Logger::FatalLog(LOCATION, "Failed to receive uint32_t data");
        this->CloseSocket();
        exit(EXIT_FAILURE);
    }
    utils::Logger::TraceLog(LOCATION, "Received data: " + std::to_string(value), this->debug_);
}

void Client::SendVector(std::vector<uint32_t> &vector) {
    // utils::Logger::WarnLog(LOCATION, "Sending vector is slow.");
    // Send data size
    std::size_t vector_size = vector.size() * sizeof(uint32_t);
    bool        is_sent     = internal::SendData(this->client_fd_, reinterpret_cast<const char *>(&vector_size), sizeof(vector_size));
    // Send array data
    is_sent &= internal::SendData(this->client_fd_, reinterpret_cast<const char *>(vector.data()), vector_size);
    if (!is_sent) {
        utils::Logger::FatalLog(LOCATION, "Failed to send vector data");
        this->CloseSocket();
        exit(EXIT_FAILURE);
    }
    this->total_bytes_sent_ += sizeof(vector_size) + vector_size;
    utils::Logger::TraceLog(LOCATION, "Sent vector: " + utils::VectorToStr(vector), this->debug_);
}

void Client::RecvVector(std::vector<uint32_t> &vector) {
    // utils::Logger::WarnLog(LOCATION, "Receiving vector is slow.");
    // Receive data size
    std::size_t vector_size = vector.size() * sizeof(uint32_t);
    bool        is_received = internal::RecvData(this->client_fd_, reinterpret_cast<char *>(&vector_size), sizeof(vector_size));
    // Receive vector data
    std::vector<uint32_t> r_vector(vector_size / sizeof(uint32_t));
    is_received &= internal::RecvData(this->client_fd_, reinterpret_cast<char *>(r_vector.data()), vector_size);
    if (!is_received) {
        utils::Logger::FatalLog(LOCATION, "Failed to receive vector data");
        this->CloseSocket();
        exit(EXIT_FAILURE);
    }
    vector = std::move(r_vector);
    utils::Logger::TraceLog(LOCATION, "Received vector: " + utils::VectorToStr(vector), this->debug_);
}

void Client::SendArray(std::array<uint32_t, 2> &array) {
    // Send array data
    bool is_sent = internal::SendData(this->client_fd_, reinterpret_cast<const char *>(array.data()), 2 * sizeof(uint32_t));
    if (!is_sent) {
        utils::Logger::FatalLog(LOCATION, "Failed to send vector data");
        this->CloseSocket();
        exit(EXIT_FAILURE);
    }
    utils::Logger::TraceLog(LOCATION, "Sent array: " + utils::ArrayToStr(array), this->debug_);
}

void Client::RecvArray(std::array<uint32_t, 2> &array) {
    // Receive vector data
    bool is_received = internal::RecvData(this->client_fd_, reinterpret_cast<char *>(array.data()), 2 * sizeof(uint32_t));
    if (!is_received) {
        utils::Logger::FatalLog(LOCATION, "Failed to receive vector data");
        this->CloseSocket();
        exit(EXIT_FAILURE);
    }
    this->total_bytes_sent_ += 2 * sizeof(uint32_t);
    utils::Logger::TraceLog(LOCATION, "Received array: " + utils::ArrayToStr(array), this->debug_);
}

void Client::SendArray(std::array<uint32_t, 4> &array) {
    // Send array data
    bool is_sent = internal::SendData(this->client_fd_, reinterpret_cast<const char *>(array.data()), 4 * sizeof(uint32_t));
    if (!is_sent) {
        utils::Logger::FatalLog(LOCATION, "Failed to send vector data");
        this->CloseSocket();
        exit(EXIT_FAILURE);
    }
    this->total_bytes_sent_ += 4 * sizeof(uint32_t);
    utils::Logger::TraceLog(LOCATION, "Sent array: " + utils::ArrayToStr(array), this->debug_);
}

void Client::RecvArray(std::array<uint32_t, 4> &array) {
    // Receive vector data
    bool is_received = internal::RecvData(this->client_fd_, reinterpret_cast<char *>(array.data()), 4 * sizeof(uint32_t));
    if (!is_received) {
        utils::Logger::FatalLog(LOCATION, "Failed to receive vector data");
        this->CloseSocket();
        exit(EXIT_FAILURE);
    }
    utils::Logger::TraceLog(LOCATION, "Received array: " + utils::ArrayToStr(array), this->debug_);
}

std::string Client::GetHostAddress() {
    return this->host_address_;
}

int Client::GetPortNumber() {
    return this->port_;
}

uint32_t Client::GetTotalBytesSent() const {
    return this->total_bytes_sent_;
}

void Client::ClearTotalBytesSent() {
    this->total_bytes_sent_ = 0;
}

}    // namespace comm
