/**
 * @file fsskey_io.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-10
 * @copyright Copyright (c) 2024
 * @brief FSS key I/O class.
 */

#ifndef INTERNAL_FSSKEY_IO_H_
#define INTERNAL_FSSKEY_IO_H_

#include <fstream>

#include "../../fss-base/dcf/distributed_comparison_function.hpp"
#include "../../fss-base/ddcf/dual_dcf.hpp"
#include "../../fss-base/dpf/distributed_point_function.hpp"
#include "../../utils/file_io.hpp"
#include "../comp/integer_comparison.hpp"
#include "../fm-index/fss_fmi.hpp"
#include "../rank/fss_rank.hpp"
#include "../zt/zero_test_dpf.hpp"

namespace fss {
namespace internal {

class FssKeyIo {
public:
    FssKeyIo(const bool debug = false, const std::string ext = ".key", const char del = ',');

    void WriteDpfKeyToFile(const std::string &file_path, const dpf::DpfKey &dpf_key, const bool is_naive = false);
    void WriteDcfKeyToFile(const std::string &file_path, const dcf::DcfKey &dcf_key);
    void WriteDdcfKeyToFile(const std::string &file_path, const ddcf::DdcfKey &ddcf_key);
    void WriteCompKeyToFile(const std::string &file_path, const comp::CompKey &comp_key);
    void WriteZeroTestKeyToFile(const std::string &file_path, const zt::ZeroTestKey &zt_key);
    void WriteFssRankKeyToFile(const std::string &file_path, const rank::FssRankKey &rank_key);
    void WriteFssFmiKeyToFile(const std::string &file_path, const fmi::FssFmiKey &fmi_key);

    void ReadDpfKeyFromFile(const std::string &file_path, const dpf::DpfParameters &params, dpf::DpfKey &dpf_key, const bool is_naive = false);
    void ReadDcfKeyFromFile(const std::string &file_path, const uint32_t n, dcf::DcfKey &dcf_key);
    void ReadDdcfKeyFromFile(const std::string &file_path, const uint32_t n, ddcf::DdcfKey &ddcf_key);
    void ReadCompKeyFromFile(const std::string &file_path, const uint32_t n, comp::CompKey &comp_key);
    void ReadZeroTestKeyFromFile(const std::string &file_path, const zt::ZeroTestParameters &params, zt::ZeroTestKey &zt_key);
    void ReadFssRankKeyFromFile(const std::string &file_path, const rank::FssRankParameters &params, rank::FssRankKey &rank_key);
    void ReadFssFmiKeyFromFile(const std::string &file_path, const fmi::FssFmiParameters &params, fmi::FssFmiKey &fmi_key);

private:
    const bool        debug_;
    const std::string ext_;
    const char        del_;
    utils::FileIo     io_;

    /**
     * @brief Read the next row from the CSV file.
     *
     * @param row Reference to a vector of strings where the read row will be stored.
     * @return True if a row was successfully read, false if the end of the file is reached.
     */
    bool ReadNextRow(std::ifstream &file, std::vector<std::string> &row);

    void ExportDpfKey(std::ofstream &file, const dpf::DpfKey &dpf_key, const bool is_naive = false);
    void ExportDcfKey(std::ofstream &file, const dcf::DcfKey &dcf_key);
    void ExportDdcfKey(std::ofstream &file, const ddcf::DdcfKey &ddcf_key);
    void ExportCompKey(std::ofstream &file, const comp::CompKey &comp_key);
    void ExportZeroTestKey(std::ofstream &file, const zt::ZeroTestKey &zt_key);
    void ExportFssRankKey(std::ofstream &file, const rank::FssRankKey &rank_key);
    void ExportFssFmiKey(std::ofstream &file, const fmi::FssFmiKey &fmi_key);

    void ImportDpfKey(std::ifstream &file, const dpf::DpfParameters &params, dpf::DpfKey &dpf_key, const bool is_naive = false);
    void ImportDcfKey(std::ifstream &file, const uint32_t n, dcf::DcfKey &dcf_key);
    void ImportDdcfKey(std::ifstream &file, const uint32_t n, ddcf::DdcfKey &ddcf_key);
    void ImportCompKey(std::ifstream &file, const uint32_t n, comp::CompKey &comp_key);
    void ImportZeroTestKey(std::ifstream &file, const zt::ZeroTestParameters &params, zt::ZeroTestKey &zt_key);
    void ImportFssRankKey(std::ifstream &file, const rank::FssRankParameters &params, rank::FssRankKey &rank_key);
    void ImportFssFmiKey(std::ifstream &file, const fmi::FssFmiParameters &params, fmi::FssFmiKey &fmi_key);
};

/**
 * @brief A class for encoding and decoding numbers using Base64 encoding.
 *
 * The `Base64Encoder` class provides methods for encoding a 64-bit unsigned integer into a Base64-encoded string
 * and decoding a Base64-encoded string back into a 64-bit unsigned integer.
 */
class Base64Encoder {
public:
    /**
     * @brief Encode a 64-bit unsigned integer into a Base64-encoded string.
     *
     * This method takes a 64-bit unsigned integer and encodes it into a Base64-encoded string.
     * The resulting Base64-encoded string may include padding characters ('=') to ensure a multiple of 4 characters in the output.
     *
     * @param number The 64-bit unsigned integer to encode.
     * @return The Base64-encoded string representing the input number.
     */
    static std::string Encode(uint64_t number);

    /**
     * @brief Decode a Base64-encoded string into a 64-bit unsigned integer.
     *
     * This method takes a Base64-encoded string and decodes it into a 64-bit unsigned integer.
     *
     * @param encoded The Base64-encoded string to decode.
     * @return The 64-bit unsigned integer decoded from the input string.
     */
    static uint64_t Decode(const std::string &encoded);
};

namespace test {

void Test_FssKeyIo( TestInfo &test_info);

}    // namespace test

}    // namespace internal
}    // namespace fss

#endif    // INTERNAL_FSSKEY_IO_H_
