/**
 * @file fss_fmi.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-25
 * @copyright Copyright (c) 2024
 * @brief FssFmi class.
 */

#ifndef FM_INDEX_FSS_FMI_H_
#define FM_INDEX_FSS_FMI_H_

#include "../rank/fss_rank.hpp"
#include "../zt/zero_test_dpf.hpp"

namespace fss {
namespace fmi {

struct FssFmiParameters {
    const uint32_t                text_bitsize;  /**< The size of the text in bits. */
    const uint32_t                text_size;     /**< The size of the text */
    const uint32_t                query_bitsize; /**< The size of the query in bits. */
    const uint32_t                query_size;    /**< The size of the query */
    const rank::FssRankParameters rank_params;   /**< The parameters for FssRank. */
    const zt::ZeroTestParameters  zt_params;     /**< The parameters for ZeroTest. */
    const bool                    debug;         /**< Debug utils::Mode flag. */
    const DebugInfo               dbg_info;      /**< Debug information. */

    /**
     * @brief Default constructor for FssFmiParameters.
     */
    FssFmiParameters();

    /**
     * @brief Parameterized constructor for FssFmiParameters.
     * @param t The size of the text in bits.
     * @param q The size of the query in bits.
     * @param dbg_info Debug information.

     */
    FssFmiParameters(const uint32_t t, const uint32_t q, const DebugInfo &dbg_info);
};

struct FssFmiKey {
    uint32_t                      rank_key_num; /**< The number of rank keys. */
    uint32_t                      zt_key_num;   /**< The number of ZeroTest keys. */
    std::vector<rank::FssRankKey> rank_keys_f;  /**< The FssRank key associated with the FssFmiKey. */
    std::vector<rank::FssRankKey> rank_keys_g;  /**< The FssRank key associated with the FssFmiKey. */
    std::vector<zt::ZeroTestKey>  zt_keys;      /**< The ZeroTest key associated with the FssFmiKey. */

    /**
     * @brief Default constructor for FssFmiKey.
     */
    FssFmiKey(){};

    /**
     * @brief Constructor for FssFmiKey with specified keys.
     * @param rank_keys_f The FssRank key associated with the FssFmiKey.
     * @param rank_keys_g The FssRank key associated with the FssFmiKey.
     * @param zt_keys The ZeroTest key associated with the FssFmiKey.
     */
    FssFmiKey(const uint32_t rank_key_num, const uint32_t zt_key_num);

    /**
     * @brief Copy constructor (deleted).
     */
    FssFmiKey(const FssFmiKey &) = delete;

    /**
     * @brief Copy assignment operator (deleted).
     */
    FssFmiKey &operator=(const FssFmiKey &) = delete;

    /**
     * @brief Move constructor (default).
     */
    FssFmiKey(FssFmiKey &&) noexcept = default;

    /**
     * @brief Move assignment operator (default).
     */
    FssFmiKey &operator=(FssFmiKey &&) noexcept = default;

    bool operator==(const FssFmiKey &rhs) const {
        return this->rank_keys_f == rhs.rank_keys_f && this->rank_keys_g == rhs.rank_keys_g && this->zt_keys == rhs.zt_keys;
    }

    bool operator!=(const FssFmiKey &rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Print the details of the FssFMI key.
     * @param rank_params The parameters for FssRank.
     * @param zt_params The parameters for ZeroTest.
     * @param debug Debug utils::Mode flag.
     */
    void PrintFssFmiKey(const FssFmiParameters &params, const bool debug) const;

    /**
     * @brief Free the resources associated with the FssFMI key.
     */
    void FreeFssFmiKey();
};

class FssFmi {
public:
    /**
     * @brief Construct a new Fss Fmi object
     * @param params The parameters for FssFmi.
     */
    FssFmi(const FssFmiParameters params);

    std::pair<FssFmiKey, FssFmiKey> GenerateKeys(const uint32_t rank_key_num, const uint32_t zt_key_num) const;

    void SetBeaverTriple(const tools::secret_sharing::bts_t &btf, const tools::secret_sharing::bts_t &btg);

    void SetSentence(const std::string &sentence);

    void Evaluate(tools::secret_sharing::Party &party, const FssFmiKey &fmi_key, const std::vector<uint32_t> &q, std::vector<uint32_t> &output) const;

private:
    const FssFmiParameters       params_;    /**< The parameters for FssFmi. */
    const rank::FssRank          rank_;      /**< The FssRank object. */
    const zt::ZeroTest           zt_;        /**< The ZeroTest object. */
    std::string                  pub_db_;    /**< The sentence for the FssFmi object. */
    uint32_t                     cf1_;       /**< The value of CF1. */
    tools::secret_sharing::bts_t btf_, btg_; /**< The Beaver triple for f and g functions. */
};

namespace test {

void Test_FssFmi(tools::secret_sharing::Party &party, TestInfo &test_info);

}    // namespace test

namespace bench {

void Bench_FssFmi(tools::secret_sharing::Party &party, const BenchInfo &bench_info);

}    // namespace bench

}    // namespace fmi
}    // namespace fss

#endif    // FM_INDEX_FSS_FMI_H_
