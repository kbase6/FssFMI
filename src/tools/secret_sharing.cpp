/**
 * @file secret_sharing.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Secret Sharing implementation.
 */

#include "secret_sharing.hpp"

#include "../utils/logger.hpp"
#include "../utils/timer.hpp"
#include "../utils/utils.hpp"
#include "random_number_generator.hpp"

#include <fstream>
#include <iostream>

namespace tools {
namespace secret_sharing {

Party::Party(const comm::CommInfo &comm_info)
    : id_(comm_info.party_id), p0_(comm::Server(comm_info.port_number, false)), p1_(comm::Client(comm_info.host_address, comm_info.port_number, false)), is_started_(false) {
}

void Party::StartCommunication(const bool debug) {
    // Clear the total bytes sent
    this->ClearTotalBytesSent();

    // Check if communication has already started
    if (this->is_started_) {
        utils::Logger::DebugLog(LOCATION, "Communication has already started.", debug);
        return;
    }

    // Start communication based on party ID
    if (id_ == 0) {
        this->p0_.Setup();
        this->p0_.Start();
    } else {
        this->p1_.Setup();
        this->p1_.Start();
    }

    // Set the flag to indicate that communication has started
    this->is_started_ = true;

    // Output communication details
    utils::Logger::DebugLog(LOCATION, "------ Communication ------", debug);
    if (this->id_ == 0) {
        utils::Logger::DebugLog(LOCATION, "- Party ID: " + std::to_string(this->id_) + "             -", debug);
        utils::Logger::DebugLog(LOCATION, "- Port number: " + std::to_string(this->p0_.GetPortNumber()) + "      -", debug);
    } else {
        utils::Logger::DebugLog(LOCATION, "- Party ID: " + std::to_string(this->id_) + "             -", debug);
        utils::Logger::DebugLog(LOCATION, "- Port number: " + std::to_string(this->p1_.GetPortNumber()) + "      -", debug);
        utils::Logger::DebugLog(LOCATION, "- Host address: " + this->p1_.GetHostAddress() + " -", debug);
    }
    utils::Logger::DebugLog(LOCATION, "---------------------------", debug);
}

void Party::EndCommunication() {
    this->p0_.CloseSocket();
    this->p1_.CloseSocket();
}

uint32_t Party::GetId() const {
    return this->id_;
}

void Party::SendRecv(uint32_t &x_0, uint32_t &x_1) {
    if (id_ == 0) {
        this->p0_.SendValue(x_0);
        this->p0_.RecvValue(x_1);
    } else {
        this->p1_.RecvValue(x_0);
        this->p1_.SendValue(x_1);
    }
}

void Party::SendRecv(std::vector<uint32_t> &x_vec_0, std::vector<uint32_t> &x_vec_1) {
    if (this->id_ == 0) {
        this->p0_.SendVector(x_vec_0);
        this->p0_.RecvVector(x_vec_1);
    } else {
        this->p1_.RecvVector(x_vec_0);
        this->p1_.SendVector(x_vec_1);
    }
}

void Party::SendRecv(std::array<uint32_t, 2> &x_arr_0, std::array<uint32_t, 2> &x_arr_1) {
    if (this->id_ == 0) {
        this->p0_.SendArray(x_arr_0);
        this->p0_.RecvArray(x_arr_1);
    } else {
        this->p1_.RecvArray(x_arr_0);
        this->p1_.SendArray(x_arr_1);
    }
}

void Party::SendRecv(std::array<uint32_t, 4> &x_arr_0, std::array<uint32_t, 4> &x_arr_1) {
    if (this->id_ == 0) {
        this->p0_.SendArray(x_arr_0);
        this->p0_.RecvArray(x_arr_1);
    } else {
        this->p1_.RecvArray(x_arr_0);
        this->p1_.SendArray(x_arr_1);
    }
}

uint32_t Party::GetTotalBytesSent() const {
    if (this->id_ == 0) {
        return this->p0_.GetTotalBytesSent();
    } else {
        return this->p1_.GetTotalBytesSent();
    }
}

uint32_t Party::OutputTotalBytesSent(const std::string &message) const {
    if (this->id_ == 0) {
        utils::Logger::InfoLog(LOCATION, "Total bytes sent" + message + "," + std::to_string(this->p0_.GetTotalBytesSent()) + ",bytes");
        return this->p0_.GetTotalBytesSent();
    } else {
        utils::Logger::InfoLog(LOCATION, "Total bytes sent" + message + "," + std::to_string(this->p1_.GetTotalBytesSent()) + ",bytes");
        return this->p1_.GetTotalBytesSent();
    }
}

void Party::ClearTotalBytesSent() {
    if (this->id_ == 0) {
        this->p0_.ClearTotalBytesSent();
    } else {
        this->p1_.ClearTotalBytesSent();
    }
}

BeaverTriplet::BeaverTriplet()
    : a(0UL), b(0UL), c(0UL) {
}

BeaverTriplet::BeaverTriplet(uint32_t val_a, uint32_t val_b, uint32_t val_c)
    : a(val_a), b(val_b), c(val_c) {
}

std::string BeaverTriplet::ToStr(const bool sup) const {
    if (sup) {
        return "(a, b, c) = (" + std::to_string(this->a) + ", " + std::to_string(this->b) + ", " + std::to_string(this->c) + ")";
    } else {
        return "(" + std::to_string(this->a) + ", " + std::to_string(this->b) + ", " + std::to_string(this->c) + ")";
    }
}

AdditiveSecretSharing::AdditiveSecretSharing()
    : bitsize_(32) {
}

AdditiveSecretSharing::AdditiveSecretSharing(uint32_t bitsize)
    : bitsize_(bitsize) {
    if (bitsize <= 1) {
        throw std::invalid_argument("The bit size must be greater than 1.");
    }
}

share_t AdditiveSecretSharing::Share(const uint32_t x) const {
    uint32_t x_0(0), x_1(0);
    x_0 = utils::Mod(rng::SecureRng::Rand64(), this->bitsize_);
    x_1 = utils::Mod(x - x_0, this->bitsize_);
    return std::make_pair(x_0, x_1);
}

uint32_t AdditiveSecretSharing::Reconst(Party &party, uint32_t x_0, uint32_t x_1) const {
    uint32_t x(0);
    party.SendRecv(x_0, x_1);
    x = utils::Mod(x_0 + x_1, this->bitsize_);
    return x;
}

shares_t AdditiveSecretSharing::Share(const std::vector<uint32_t> &x_vec) const {
    size_t                length = x_vec.size();
    std::vector<uint32_t> x_vec_0(length);
    std::vector<uint32_t> x_vec_1(length);
    for (size_t i = 0; i < length; i++) {
        x_vec_0[i] = utils::Mod(rng::SecureRng::Rand64(), this->bitsize_);
        x_vec_1[i] = utils::Mod(x_vec[i] - x_vec_0[i], this->bitsize_);
    }
    return std::make_pair(x_vec_0, x_vec_1);
}

void AdditiveSecretSharing::Reconst(Party &party, std::vector<uint32_t> &x_vec_0, std::vector<uint32_t> &x_vec_1, std::vector<uint32_t> &output) const {
    size_t length = output.size();
    party.SendRecv(x_vec_0, x_vec_1);
    for (size_t i = 0; i < length; i++) {
        output[i] = utils::Mod(x_vec_0[i] + x_vec_1[i], this->bitsize_);
    }
}

void AdditiveSecretSharing::Reconst(Party &party, std::array<uint32_t, 2> &x_arr_0, std::array<uint32_t, 2> &x_arr_1, std::array<uint32_t, 2> &output) const {
    size_t length = output.size();
    party.SendRecv(x_arr_0, x_arr_1);
    for (size_t i = 0; i < length; i++) {
        output[i] = utils::Mod(x_arr_0[i] + x_arr_1[i], this->bitsize_);
    }
}

void AdditiveSecretSharing::Reconst(Party &party, std::array<uint32_t, 4> &x_arr_0, std::array<uint32_t, 4> &x_arr_1, std::array<uint32_t, 4> &output) const {
    size_t length = output.size();
    party.SendRecv(x_arr_0, x_arr_1);
    for (size_t i = 0; i < length; i++) {
        output[i] = utils::Mod(x_arr_0[i] + x_arr_1[i], this->bitsize_);
    }
}

void AdditiveSecretSharing::GenerateBeaverTriples(const uint32_t bt_num, bts_t &bt_vec) const {
    for (uint32_t i = 0; i < bt_num; i++) {
        uint32_t val_a = utils::Mod(rng::SecureRng::Rand64(), this->bitsize_);
        uint32_t val_b = utils::Mod(rng::SecureRng::Rand64(), this->bitsize_);
        uint32_t val_c = utils::Mod(val_a * val_b, this->bitsize_);
        bt_vec[i]      = BeaverTriplet(val_a, val_b, val_c);
    }
}

std::pair<bts_t, bts_t> AdditiveSecretSharing::ShareBeaverTriples(const bts_t &bt_vec) const {
    bts_t bt_vec_0(bt_vec.size());
    bts_t bt_vec_1(bt_vec.size());
    for (size_t i = 0; i < bt_vec.size(); i++) {
        bt_vec_0[i].a = utils::Mod(rng::SecureRng::Rand64(), this->bitsize_);
        bt_vec_1[i].a = utils::Mod(bt_vec[i].a - bt_vec_0[i].a, this->bitsize_);
        bt_vec_0[i].b = utils::Mod(rng::SecureRng::Rand64(), this->bitsize_);
        bt_vec_1[i].b = utils::Mod(bt_vec[i].b - bt_vec_0[i].b, this->bitsize_);
        bt_vec_0[i].c = utils::Mod(rng::SecureRng::Rand64(), this->bitsize_);
        bt_vec_1[i].c = utils::Mod(bt_vec[i].c - bt_vec_0[i].c, this->bitsize_);
    }
    return std::make_pair(bt_vec_0, bt_vec_1);
}

uint32_t AdditiveSecretSharing::Mult(Party &party, const BeaverTriplet &bt, const uint32_t x, const uint32_t y) const {
    uint32_t                z;
    std::array<uint32_t, 2> de{0, 0}, de_0{0, 0}, de_1{0, 0};
    // Calculate the differences de_0 and de_1 based on party_id.
    if (party.GetId() == 0) {
        de_0[0] = utils::Mod(x - bt.a, this->bitsize_);
        de_0[1] = utils::Mod(y - bt.b, this->bitsize_);
    } else {
        de_1[0] = utils::Mod(x - bt.a, this->bitsize_);
        de_1[1] = utils::Mod(y - bt.b, this->bitsize_);
    }
    // Calculate the final differences de based on de_0 and de_1.
    Reconst(party, de_0, de_1, de);
    // Calculate the secure multiplication result based on party_id.
    if (party.GetId() == 0) {
        z = utils::Mod((de[1] * bt.a) + (de[0] * bt.b) + bt.c + (de[0] * de[1]), this->bitsize_);
    } else {
        z = utils::Mod((de[1] * bt.a) + (de[0] * bt.b) + bt.c, this->bitsize_);
    }
    return z;
}

std::array<uint32_t, 2> AdditiveSecretSharing::Mult2(Party &party, const BeaverTriplet &bt1, const BeaverTriplet &bt2, const uint32_t x1, const uint32_t y1, const uint32_t x2, const uint32_t y2) const {
    std::array<uint32_t, 2> z;
    std::array<uint32_t, 4> de{0, 0, 0, 0}, de_0{0, 0, 0, 0}, de_1{0, 0, 0, 0};
    // Calculate the differences de_0 and de_1 based on party_id.
    if (party.GetId() == 0) {
        de_0[0] = utils::Mod(x1 - bt1.a, this->bitsize_);
        de_0[1] = utils::Mod(y1 - bt1.b, this->bitsize_);
        de_0[2] = utils::Mod(x2 - bt2.a, this->bitsize_);
        de_0[3] = utils::Mod(y2 - bt2.b, this->bitsize_);
    } else {
        de_1[0] = utils::Mod(x1 - bt1.a, this->bitsize_);
        de_1[1] = utils::Mod(y1 - bt1.b, this->bitsize_);
        de_1[2] = utils::Mod(x2 - bt2.a, this->bitsize_);
        de_1[3] = utils::Mod(y2 - bt2.b, this->bitsize_);
    }
    // Calculate the final differences de based on de_0 and de_1.
    Reconst(party, de_0, de_1, de);
    // Calculate the secure multiplication result based on party_id.
    if (party.GetId() == 0) {
        z[0] = utils::Mod((de[1] * bt1.a) + (de[0] * bt1.b) + bt1.c + (de[0] * de[1]), this->bitsize_);
        z[1] = utils::Mod((de[3] * bt2.a) + (de[2] * bt2.b) + bt2.c + (de[2] * de[3]), this->bitsize_);
    } else {
        z[0] = utils::Mod((de[1] * bt1.a) + (de[0] * bt1.b) + bt1.c, this->bitsize_);
        z[1] = utils::Mod((de[3] * bt2.a) + (de[2] * bt2.b) + bt2.c, this->bitsize_);
    }
    return z;
}

void AdditiveSecretSharing::Mult(Party &party, const bts_t &bt_vec, const std::vector<uint32_t> &x_vec, const std::vector<uint32_t> &y_vec, std::vector<uint32_t> &z_vec) const {
    size_t                num = z_vec.size();
    std::vector<uint32_t> de_vec(num * 2), de_vec_0(num * 2), de_vec_1(num * 2);
    for (size_t i = 0; i < num; i++) {
        // Calculate the differences de_0 and de_1 based on party_id.
        if (party.GetId() == 0) {
            de_vec_0[2 * i]     = utils::Mod(x_vec[i] - bt_vec[i].a, this->bitsize_);
            de_vec_0[2 * i + 1] = utils::Mod(y_vec[i] - bt_vec[i].b, this->bitsize_);
        } else {
            de_vec_1[2 * i]     = utils::Mod(x_vec[i] - bt_vec[i].a, this->bitsize_);
            de_vec_1[2 * i + 1] = utils::Mod(y_vec[i] - bt_vec[i].b, this->bitsize_);
        }
    }
    // Calculate the final differences de based on de_0 and de_1.
    Reconst(party, de_vec_0, de_vec_1, de_vec);
    for (size_t i = 0; i < num; i++) {
        // Calculate the secure multiplication result based on party_id.
        if (party.GetId() == 0) {
            z_vec[i] = utils::Mod((de_vec[2 * i + 1] * bt_vec[i].a) + (de_vec[2 * i] * bt_vec[i].b) + bt_vec[i].c + (de_vec[2 * i] * de_vec[2 * i + 1]), this->bitsize_);
        } else {
            z_vec[i] = utils::Mod((de_vec[2 * i + 1] * bt_vec[i].a) + (de_vec[2 * i] * bt_vec[i].b) + bt_vec[i].c, this->bitsize_);
        }
    }
}

share_t BooleanSecretSharing::Share(const uint32_t x) const {
    uint32_t x_0(0), x_1(0);
    x_0 = rng::SecureRng::RandBool();
    x_1 = x ^ x_0;
    return std::make_pair(x_0, x_1);
}

uint32_t BooleanSecretSharing::Reconst(Party &party, uint32_t x_0, uint32_t x_1) const {
    uint32_t x(0);
    party.SendRecv(x_0, x_1);
    x = x_0 ^ x_1;
    return x;
}

shares_t BooleanSecretSharing::Share(const std::vector<uint32_t> &x_vec) const {
    size_t                length = x_vec.size();
    std::vector<uint32_t> x_vec_0(length);
    std::vector<uint32_t> x_vec_1(length);
    for (size_t i = 0; i < length; i++) {
        x_vec_0[i] = rng::SecureRng::RandBool();
        x_vec_1[i] = x_vec[i] ^ x_vec_0[i];
    }
    return std::make_pair(x_vec_0, x_vec_1);
}

void BooleanSecretSharing::Reconst(Party &party, std::vector<uint32_t> &x_vec_0, std::vector<uint32_t> &x_vec_1, std::vector<uint32_t> &output) const {
    size_t length = output.size();
    party.SendRecv(x_vec_0, x_vec_1);
    for (size_t i = 0; i < length; i++) {
        output[i] = x_vec_0[i] ^ x_vec_1[i];
    }
}

void BooleanSecretSharing::Reconst(Party &party, std::array<uint32_t, 2> &x_arr_0, std::array<uint32_t, 2> &x_arr_1, std::array<uint32_t, 2> &output) const {
    size_t length = output.size();
    party.SendRecv(x_arr_0, x_arr_1);
    for (size_t i = 0; i < length; i++) {
        output[i] = x_arr_0[i] ^ x_arr_1[i];
    }
}

void BooleanSecretSharing::Reconst(Party &party, std::array<uint32_t, 4> &x_arr_0, std::array<uint32_t, 4> &x_arr_1, std::array<uint32_t, 4> &output) const {
    size_t length = output.size();
    party.SendRecv(x_arr_0, x_arr_1);
    for (size_t i = 0; i < length; i++) {
        output[i] = x_arr_0[i] ^ x_arr_1[i];
    }
}

void BooleanSecretSharing::GenerateBeaverTriples(const uint32_t bt_num, bts_t &bt_vec) const {
    for (uint32_t i = 0; i < bt_num; i++) {
        uint32_t val_a = rng::SecureRng::RandBool();
        uint32_t val_b = rng::SecureRng::RandBool();
        uint32_t val_c = val_a & val_b;
        bt_vec[i]      = BeaverTriplet(val_a, val_b, val_c);
    }
}

std::pair<bts_t, bts_t> BooleanSecretSharing::ShareBeaverTriples(const bts_t &bt_vec) const {
    bts_t bt_vec_0(bt_vec.size());
    bts_t bt_vec_1(bt_vec.size());
    for (size_t i = 0; i < bt_vec.size(); i++) {
        bt_vec_0[i].a = rng::SecureRng::RandBool();
        bt_vec_1[i].a = bt_vec[i].a ^ bt_vec_0[i].a;
        bt_vec_0[i].b = rng::SecureRng::RandBool();
        bt_vec_1[i].b = bt_vec[i].b ^ bt_vec_0[i].b;
        bt_vec_0[i].c = rng::SecureRng::RandBool();
        bt_vec_1[i].c = bt_vec[i].c ^ bt_vec_0[i].c;
    }
    return std::make_pair(bt_vec_0, bt_vec_1);
}

uint32_t BooleanSecretSharing::And(Party &party, const BeaverTriplet &bt_b, const uint32_t x_b, const uint32_t y_b) const {
    uint32_t                z_b;
    std::array<uint32_t, 2> de{0, 0}, de_0{0, 0}, de_1{0, 0};
    // Calculate the differences de_0 and de_1 based on party_id.
    if (party.GetId() == 0) {
        de_0[0] = x_b ^ bt_b.a;
        de_0[1] = y_b ^ bt_b.b;
    } else {
        de_1[0] = x_b ^ bt_b.a;
        de_1[1] = y_b ^ bt_b.b;
    }
    // Calculate the final differences de based on de_0 and de_1.
    Reconst(party, de_0, de_1, de);
    // Calculate the secure multiplication result based on party_id.
    if (party.GetId() == 0) {
        z_b = (de[1] & bt_b.a) ^ (de[0] & bt_b.b) ^ bt_b.c ^ (de[0] & de[1]);
    } else {
        z_b = (de[1] & bt_b.a) ^ (de[0] & bt_b.b) ^ bt_b.c;
    }
    return z_b;
}

void BooleanSecretSharing::And(Party &party, const bts_t &btb_vec, const std::vector<uint32_t> &xb_vec, const std::vector<uint32_t> &yb_vec, std::vector<uint32_t> &zb_vec) const {
    size_t                num = zb_vec.size();
    std::vector<uint32_t> de_vec(num * 2), de_vec_0(num * 2), de_vec_1(num * 2);
    for (size_t i = 0; i < num; i++) {
        // Calculate the differences de_0 and de_1 based on party_id.
        if (party.GetId() == 0) {
            de_vec_0[2 * i]     = xb_vec[i] ^ btb_vec[i].a;
            de_vec_0[2 * i + 1] = yb_vec[i] ^ btb_vec[i].b;
        } else {
            de_vec_1[2 * i]     = xb_vec[i] ^ btb_vec[i].a;
            de_vec_1[2 * i + 1] = yb_vec[i] ^ btb_vec[i].b;
        }
    }
    // Calculate the final differences de based on de_0 and de_1.
    Reconst(party, de_vec_0, de_vec_1, de_vec);
    for (size_t i = 0; i < num; i++) {
        // Calculate the secure multiplication result based on party_id.
        if (party.GetId() == 0) {
            zb_vec[i] = (de_vec[2 * i + 1] & btb_vec[i].a) ^ (de_vec[2 * i] & btb_vec[i].b) ^ btb_vec[i].c ^ (de_vec[2 * i] & de_vec[2 * i + 1]);
        } else {
            zb_vec[i] = (de_vec[2 * i + 1] & btb_vec[i].a) ^ (de_vec[2 * i] & btb_vec[i].b) ^ btb_vec[i].c;
        }
    }
}

uint32_t BooleanSecretSharing::Or(Party &party, const BeaverTriplet &bt_b, const uint32_t x_b, const uint32_t y_b) const {
    uint32_t nx_b, ny_b, zb_0, zb_1;
    if (party.GetId() == 0) {
        nx_b = x_b ^ 1;
        ny_b = y_b ^ 1;
        zb_0 = And(party, bt_b, nx_b, ny_b);
        zb_0 = zb_0 ^ 1;
        return zb_0;
    } else {
        zb_1 = And(party, bt_b, x_b, y_b);
        return zb_1;
    }
}

void BooleanSecretSharing::Or(Party &party, const bts_t &btb_vec, const std::vector<uint32_t> &xb_vec, const std::vector<uint32_t> &yb_vec, std::vector<uint32_t> &zb_vec) const {
    size_t                num = zb_vec.size();
    std::vector<uint32_t> nxb_vec(num), nyb_vec(num);
    if (party.GetId() == 0) {
        for (size_t i = 0; i < num; i++) {
            nxb_vec[i] = xb_vec[i] ^ 1;
            nyb_vec[i] = yb_vec[i] ^ 1;
        }
        And(party, btb_vec, nxb_vec, nyb_vec, zb_vec);
        for (size_t i = 0; i < num; i++) {
            zb_vec[i] = zb_vec[i] ^ 1;
        }
    } else {
        And(party, btb_vec, xb_vec, yb_vec, zb_vec);
    }
}

ShareHandler::ShareHandler(const bool debug, const bool io_debug, const std::string ext)
    : debug_(debug), io_(io_debug, ext) {
}

void ShareHandler::ExportShare(const std::string &file_path_p0, const std::string &file_path_p1, std::pair<uint32_t, uint32_t> &x_sh) {
    this->io_.WriteValueToFile(file_path_p0, x_sh.first);
    this->io_.WriteValueToFile(file_path_p1, x_sh.second);
}

void ShareHandler::ExportShare(const std::string &file_path_p0, const std::string &file_path_p1, shares_t &x_vec_sh) {
    this->io_.WriteVectorToFile(file_path_p0, x_vec_sh.first);
    this->io_.WriteVectorToFile(file_path_p1, x_vec_sh.second);
}

void ShareHandler::LoadShare(const std::string &file_path, uint32_t &x_sh) {
    this->io_.ReadValueFromFile(file_path, x_sh);
}

void ShareHandler::LoadShare(const std::string &file_path, std::vector<uint32_t> &x_vec_sh) {
    this->io_.ReadVectorFromFile(file_path, x_vec_sh);
}

void ShareHandler::ExportBT(const std::string &file_path, bts_t &bt_vec) {
    this->WriteBeaverTriplesToFile(file_path, bt_vec);
}

void ShareHandler::ExportBTShare(const std::string &file_path_p0, const std::string &file_path_p1, std::pair<bts_t, bts_t> &bt_vec_sh) {
    this->WriteBeaverTriplesToFile(file_path_p0, bt_vec_sh.first);
    this->WriteBeaverTriplesToFile(file_path_p1, bt_vec_sh.second);
}

void ShareHandler::LoadBTShare(const std::string &file_path, bts_t &bt_vec_sh) {
    this->ReadBeaverTriplesFromFile(file_path, bt_vec_sh);
}

void ShareHandler::WriteBeaverTriplesToFile(const std::string &file_path, bts_t &bt_vec) {
    // Open the file
    std::ofstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }
    // Write the beaver triples to the file
    file << bt_vec.size() << "\n";
    for (size_t i = 0; i < bt_vec.size(); i++) {
        file << bt_vec[i].a << "," << bt_vec[i].b << "," << bt_vec[i].c << "\n";
    }
    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "Beaver Triple have been written to the file (" + file_path + ")", this->debug_);
}

void ShareHandler::ReadBeaverTriplesFromFile(const std::string &file_path, bts_t &bt_vec) {
    // Open the file
    std::ifstream file;
    if (this->io_.OpenFile(file, file_path, LOCATION)) {
        // Read the number of elements from the first line of the file
        uint32_t size = this->io_.ReadNumCountFromFile(file, LOCATION);
        bts_t    bts;
        bts.reserve(size);
        for (uint32_t i = 0; i < size; i++) {
            std::string line;
            if (std::getline(file, line)) {
                std::vector<uint32_t> vec;
                io_.SplitStringToUint32(line, vec);
                bts.push_back(BeaverTriplet(vec[0], vec[1], vec[2]));
            }
        }
        // Close the file
        file.close();
        bt_vec = std::move(bts);
    }
}

}    // namespace secret_sharing
}    // namespace tools
