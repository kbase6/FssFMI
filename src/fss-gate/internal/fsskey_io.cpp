/**
 * @file fsskey_io.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-10
 * @copyright Copyright (c) 2024
 * @brief FSS key I/O implementation
 */

#include "fsskey_io.hpp"

#include <cstring>
#include <fstream>
#include <sstream>

#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"

namespace {

const char base64_chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * @brief Convert a 64-bit unsigned integer into a vector of bytes.
 *
 * This private method converts a 64-bit unsigned integer into a vector of bytes.
 *
 * @param number The 64-bit unsigned integer to convert.
 * @return A vector of bytes representing the input number.
 */
std::vector<unsigned char> ConvertUInt64ToBytes(uint64_t number) {
    std::vector<unsigned char> bytes;
    while (number > 0) {
        bytes.push_back(static_cast<unsigned char>(number & 0xFF));
        number >>= 8;
    }
    return bytes;
}

/**
 * @brief Convert a vector of bytes into a 64-bit unsigned integer.
 *
 * This private method converts a vector of bytes back into a 64-bit unsigned integer.
 *
 * @param bytes The vector of bytes to convert.
 * @return The 64-bit unsigned integer decoded from the input bytes.
 */
uint64_t ConvertBytesToUInt64(const std::vector<unsigned char> &bytes) {
    uint64_t result = 0;
    for (int i = bytes.size() - 1; i >= 0; i--) {
        result <<= 8;
        result |= bytes[i];
    }
    return result;
}

/**
 * @brief Parses a string line into a vector of strings using a delimiter.
 *
 * Parses the given string 'line' into a vector of strings using the specified 'del' delimiter.
 * Splits the 'line' into separate elements based on the delimiter and returns the resulting vector.
 *
 * @param line The input string to be parsed.
 * @param del The delimiter character used to split the string.
 * @return A vector of strings containing the parsed elements.
 */
std::vector<std::string> ParseRow(const std::string &line, const char del) {
    std::vector<std::string> row;
    std::stringstream        ss(line);
    std::string              cell;
    while (std::getline(ss, cell, del)) {
        row.push_back(cell);
    }
    return row;
}

/**
 * @brief Convert a string to a boolean value.
 *
 * This function interprets the input string, returning true if the string is not "0" (non-zero), and false if it is "0."
 *
 * @param s The string to convert to a boolean.
 * @return True if the string is not "0," false if it is "0."
 */
bool StrToBool(std::string const &s) {
    return s != "0";
}

}    // namespace

namespace fss {
namespace internal {

FssKeyIo::FssKeyIo(const bool debug, const std::string ext, const char del)
    : debug_(debug), ext_(ext), del_(del), io_(debug_, ext) {
}

void FssKeyIo::WriteDpfKeyToFile(const std::string &file_path, const dpf::DpfKey &dpf_key, const bool is_naive) {
    // Open the file
    std::ofstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ExportDpfKey(file, dpf_key, is_naive);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "DPF key has been written to the file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::WriteDcfKeyToFile(const std::string &file_path, const dcf::DcfKey &dcf_key) {
    // Open the file
    std::ofstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ExportDcfKey(file, dcf_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "DCF key has been written to the file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::WriteDdcfKeyToFile(const std::string &file_path, const ddcf::DdcfKey &ddcf_key) {
    // Open the file
    std::ofstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ExportDdcfKey(file, ddcf_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "DDCF key has been written to the file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::WriteCompKeyToFile(const std::string &file_path, const comp::CompKey &comp_key) {
    // Open the file
    std::ofstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ExportCompKey(file, comp_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "COMP key has been written to the file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::WriteZeroTestKeyToFile(const std::string &file_path, const zt::ZeroTestKey &zt_key) {
    // Open the file
    std::ofstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ExportZeroTestKey(file, zt_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "Zero test key has been written to the file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::WriteFssRankKeyToFile(const std::string &file_path, const rank::FssRankKey &rank_key) {
    // Open the file
    std::ofstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ExportFssRankKey(file, rank_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "FSS rank key has been written to the file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::WriteFssFmiKeyToFile(const std::string &file_path, const fmi::FssFmiKey &fmi_key) {
    // Open the file
    std::ofstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ExportFssFmiKey(file, fmi_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "FSS FMI key has been written to the file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::ReadDpfKeyFromFile(const std::string &file_path, const dpf::DpfParameters &params, dpf::DpfKey &dpf_key, const bool is_naive) {
    // Open the file for reading
    std::ifstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ImportDpfKey(file, params, dpf_key, is_naive);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "DPF key read from file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::ReadDcfKeyFromFile(const std::string &file_path, const uint32_t n, dcf::DcfKey &dcf_key) {
    // Open the file for reading
    std::ifstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ImportDcfKey(file, n, dcf_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "DCF key read from file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::ReadDdcfKeyFromFile(const std::string &file_path, const uint32_t n, ddcf::DdcfKey &ddcf_key) {
    // Open the file for reading
    std::ifstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ImportDdcfKey(file, n, ddcf_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "DDCF key read from file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::ReadCompKeyFromFile(const std::string &file_path, const uint32_t n, comp::CompKey &comp_key) {
    // Open the file for reading
    std::ifstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ImportCompKey(file, n, comp_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "DDCF key read from file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::ReadZeroTestKeyFromFile(const std::string &file_path, const zt::ZeroTestParameters &params, zt::ZeroTestKey &zt_key) {
    // Open the file for reading
    std::ifstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ImportZeroTestKey(file, params, zt_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "Zero test key read from file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::ReadFssRankKeyFromFile(const std::string &file_path, const rank::FssRankParameters &params, rank::FssRankKey &rank_key) {
    // Open the file for reading
    std::ifstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ImportFssRankKey(file, params, rank_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "FSS rank key read from file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::ReadFssFmiKeyFromFile(const std::string &file_path, const fmi::FssFmiParameters &params, fmi::FssFmiKey &fmi_key) {
    // Open the file for reading
    std::ifstream file;
    if (!this->io_.OpenFile(file, file_path, LOCATION)) {
        exit(EXIT_FAILURE);
    }

    this->ImportFssFmiKey(file, params, fmi_key);

    // Close the file
    file.close();
    utils::Logger::DebugLog(LOCATION, "FSS FMI key read from file (" + file_path + this->ext_ + ")", this->debug_);
}

void FssKeyIo::ExportDpfKey(std::ofstream &file, const dpf::DpfKey &dpf_key, const bool is_naive) {
    file << dpf_key.party_id << std::endl;
    file << Base64Encoder::Encode(dpf_key.init_seed.GetHigh()) << this->del_ << Base64Encoder::Encode(dpf_key.init_seed.GetLow()) << std::endl;
    for (uint32_t i = 0; i < dpf_key.cw_length; i++) {
        file << Base64Encoder::Encode(dpf_key.correction_words[i].seed.GetHigh()) << this->del_
             << Base64Encoder::Encode(dpf_key.correction_words[i].seed.GetLow()) << this->del_
             << dpf_key.correction_words[i].control_left << this->del_
             << dpf_key.correction_words[i].control_right << std::endl;
    }
    file << Base64Encoder::Encode(dpf_key.output.GetHigh()) << this->del_ << Base64Encoder::Encode(dpf_key.output.GetLow()) << std::endl;
}

void FssKeyIo::ExportDcfKey(std::ofstream &file, const dcf::DcfKey &dcf_key) {
    file << dcf_key.party_id << std::endl;
    file << Base64Encoder::Encode(dcf_key.init_seed.GetHigh()) << this->del_ << Base64Encoder::Encode(dcf_key.init_seed.GetLow()) << std::endl;
    for (uint32_t i = 0; i < dcf_key.cw_length; i++) {
        file << Base64Encoder::Encode(dcf_key.correction_words[i].seed.GetHigh()) << this->del_
             << Base64Encoder::Encode(dcf_key.correction_words[i].seed.GetLow()) << this->del_
             << dcf_key.correction_words[i].control_left << this->del_
             << dcf_key.correction_words[i].control_right << this->del_
             << dcf_key.correction_words[i].value << std::endl;
    }
    file << dcf_key.output << std::endl;
}

void FssKeyIo::ExportDdcfKey(std::ofstream &file, const ddcf::DdcfKey &ddcf_key) {
    this->ExportDcfKey(file, ddcf_key.dcf_key);
    file << ddcf_key.mask << std::endl;
}

void FssKeyIo::ExportCompKey(std::ofstream &file, const comp::CompKey &comp_key) {
    this->ExportDdcfKey(file, comp_key.ddcf_key);
    file << comp_key.shr1_in << this->del_
         << comp_key.shr2_in << this->del_
         << comp_key.shr_out << std::endl;
}

void FssKeyIo::ExportZeroTestKey(std::ofstream &file, const zt::ZeroTestKey &zt_key) {
    this->ExportDpfKey(file, zt_key.dpf_key, false);
    file << zt_key.shr_in << std::endl;
}

void FssKeyIo::ExportFssRankKey(std::ofstream &file, const rank::FssRankKey &rank_key) {
    this->ExportDpfKey(file, rank_key.dpf_key);
    file << rank_key.shr_in << std::endl;
}

void FssKeyIo::ExportFssFmiKey(std::ofstream &file, const fmi::FssFmiKey &fmi_key) {
    for (uint32_t i = 0; i < fmi_key.rank_key_num; i++) {
        this->ExportFssRankKey(file, fmi_key.rank_keys_f[i]);
        this->ExportFssRankKey(file, fmi_key.rank_keys_g[i]);
    }

    for (uint32_t i = 0; i < fmi_key.zt_key_num; i++) {
        this->ExportZeroTestKey(file, fmi_key.zt_keys[i]);
    }
}

void FssKeyIo::ImportDpfKey(std::ifstream &file, const dpf::DpfParameters &params, dpf::DpfKey &dpf_key, const bool is_naive) {
    dpf::DpfKey key;
    key.Initialize(params, 0, is_naive);
    std::vector<std::string> row;
    if (this->ReadNextRow(file, row)) {
        key.party_id = std::stoul(row[0]);
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read party id");
    }
    if (this->ReadNextRow(file, row)) {
        key.init_seed = fss::Block(Base64Encoder::Decode(row[0]), Base64Encoder::Decode(row[1]));
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read seed");
    }
    uint32_t cw_length = is_naive ? params.input_bitsize : params.terminate_bitsize;
    for (uint32_t i = 0; i < cw_length; i++) {
        if (this->ReadNextRow(file, row)) {
            key.correction_words[i].seed          = Block(Base64Encoder::Decode(row[0]), Base64Encoder::Decode(row[1]));
            key.correction_words[i].control_left  = StrToBool(row[2]);
            key.correction_words[i].control_right = StrToBool(row[3]);
        } else {
            utils::Logger::ErrorLog(LOCATION, "Failed to read correction word");
        }
    }
    if (this->ReadNextRow(file, row)) {
        key.output = Block(Base64Encoder::Decode(row[0]), Base64Encoder::Decode(row[1]));
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read output");
    }
    dpf_key = std::move(key);
}

void FssKeyIo::ImportDcfKey(std::ifstream &file, const uint32_t n, dcf::DcfKey &dcf_key) {
    dcf::DcfKey key;
    key.Initialize(n, 0);
    std::vector<std::string> row;
    if (this->ReadNextRow(file, row)) {
        key.party_id = std::stoul(row[0]);
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read party id");
    }
    if (this->ReadNextRow(file, row)) {
        key.init_seed = Block(Base64Encoder::Decode(row[0]), Base64Encoder::Decode(row[1]));
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read seed");
    }
    for (uint32_t i = 0; i < n; i++) {
        if (this->ReadNextRow(file, row)) {
            key.correction_words[i].seed          = Block(Base64Encoder::Decode(row[0]), Base64Encoder::Decode(row[1]));
            key.correction_words[i].control_left  = StrToBool(row[2]);
            key.correction_words[i].control_right = StrToBool(row[3]);
            key.correction_words[i].value         = std::stoul(row[4]);
        } else {
            utils::Logger::ErrorLog(LOCATION, "Failed to read correction word");
        }
    }
    if (this->ReadNextRow(file, row)) {
        key.output = std::stoul(row[0]);
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read output");
    }
    dcf_key = std::move(key);
}

void FssKeyIo::ImportDdcfKey(std::ifstream &file, const uint32_t n, ddcf::DdcfKey &ddcf_key) {
    ddcf::DdcfKey key;
    this->ImportDcfKey(file, n, key.dcf_key);

    std::vector<std::string> row;
    if (this->ReadNextRow(file, row)) {
        key.mask = std::stoul(row[0]);
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read mask");
    }
    ddcf_key = std::move(key);
}

void FssKeyIo::ImportCompKey(std::ifstream &file, const uint32_t n, comp::CompKey &comp_key) {
    comp::CompKey key;
    this->ImportDdcfKey(file, n - 1, key.ddcf_key);

    std::vector<std::string> row;
    if (this->ReadNextRow(file, row)) {
        key.shr1_in = std::stoul(row[0]);
        key.shr2_in = std::stoul(row[1]);
        key.shr_out = std::stoul(row[2]);
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read share of r_in, r_out");
    }
    comp_key = std::move(key);
}

void FssKeyIo::ImportZeroTestKey(std::ifstream &file, const zt::ZeroTestParameters &params, zt::ZeroTestKey &zt_key) {
    zt::ZeroTestKey key;
    this->ImportDpfKey(file, dpf::DpfParameters(params.input_bitsize, params.element_bitsize, params.dbg_info), key.dpf_key, false);

    std::vector<std::string> row;
    if (this->ReadNextRow(file, row)) {
        key.shr_in = std::stoul(row[0]);
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read share of r_in");
    }
    zt_key = std::move(key);
}

void FssKeyIo::ImportFssRankKey(std::ifstream &file, const rank::FssRankParameters &params, rank::FssRankKey &rank_key) {
    rank::FssRankKey key;
    dpf::DpfKey      dpf_key;
    this->ImportDpfKey(file, params.dpf_params, key.dpf_key);

    std::vector<std::string> row;
    if (this->ReadNextRow(file, row)) {
        key.shr_in = std::stoul(row[0]);
    } else {
        utils::Logger::ErrorLog(LOCATION, "Failed to read share of r_in");
    }
    rank_key = std::move(key);
}

void FssKeyIo::ImportFssFmiKey(std::ifstream &file, const fmi::FssFmiParameters &params, fmi::FssFmiKey &fmi_key) {
    fmi::FssFmiKey key{params.query_size - 1, params.query_size};
    for (uint32_t i = 0; i < key.rank_key_num; i++) {
        rank::FssRankKey rank_key_f, rank_key_g;
        this->ImportFssRankKey(file, params.rank_params, rank_key_f);
        this->ImportFssRankKey(file, params.rank_params, rank_key_g);
        key.rank_keys_f.push_back(std::move(rank_key_f));
        key.rank_keys_g.push_back(std::move(rank_key_g));
    }

    for (uint32_t i = 0; i < key.zt_key_num; i++) {
        zt::ZeroTestKey zt_key;
        this->ImportZeroTestKey(file, params.zt_params, zt_key);
        key.zt_keys.push_back(std::move(zt_key));
    }
    fmi_key = std::move(key);
}

bool FssKeyIo::ReadNextRow(std::ifstream &file, std::vector<std::string> &row) {
    std::string line;
    if (std::getline(file, line)) {
        row = ParseRow(line, this->del_);
        return true;
    }
    return false;
}

std::string Base64Encoder::Encode(uint64_t number) {
    std::string result;
    uint32_t    padding = 0;

    // Convert the uint64_t number to bytes
    std::vector<unsigned char> bytes = ConvertUInt64ToBytes(number);

    // Calculate padding
    uint32_t mod = bytes.size() % 3;
    if (mod > 0) {
        padding = 3 - mod;
        bytes.resize(bytes.size() + padding, 0);
    }

    // Encode the bytes to Base64
    for (size_t i = 0; i < bytes.size(); i += 3) {
        uint32_t combined = (static_cast<uint32_t>(bytes[i]) << 16) |
                            (static_cast<uint32_t>(bytes[i + 1]) << 8) |
                            static_cast<uint32_t>(bytes[i + 2]);

        result.push_back(base64_chars[(combined >> 18) & 0x3F]);
        result.push_back(base64_chars[(combined >> 12) & 0x3F]);
        result.push_back(base64_chars[(combined >> 6) & 0x3F]);
        result.push_back(base64_chars[combined & 0x3F]);
    }

    // Add padding characters if necessary
    for (uint32_t i = 0; i < padding; i++) {
        result.push_back('=');
    }

    return result;
}

uint64_t Base64Encoder::Decode(const std::string &encoded) {
    std::vector<uint32_t> values;

    // Convert Base64 characters to their values
    for (char c : encoded) {
        if (c == '=')
            break;

        const char *found = std::strchr(base64_chars, c);
        if (found != nullptr) {
            values.push_back(static_cast<uint32_t>(found - base64_chars));
        }
    }

    // Combine values to bytes
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < values.size(); i += 4) {
        uint32_t combined = (values[i] << 18) |
                            (values[i + 1] << 12) |
                            (values[i + 2] << 6) |
                            values[i + 3];

        bytes.push_back(static_cast<unsigned char>((combined >> 16) & 0xFF));
        bytes.push_back(static_cast<unsigned char>((combined >> 8) & 0xFF));
        bytes.push_back(static_cast<unsigned char>(combined & 0xFF));
    }

    // Convert bytes to uint64_t
    return ConvertBytesToUInt64(bytes);
}

}    // namespace internal
}    // namespace fss
