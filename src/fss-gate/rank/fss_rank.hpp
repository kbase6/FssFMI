/**
 * @file fss_rank.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-24
 * @copyright Copyright (c) 2024
 * @brief FssRank class.
 */

#ifndef RANK_FSS_RANK_H_
#define RANK_FSS_RANK_H_

#include "../../fss-base/dpf/distributed_point_function.hpp"
#include "../../tools/secret_sharing.hpp"

namespace fss {
namespace rank {

struct FssRankParameters {
    const uint32_t           text_bitsize; /**< The size of the text in bits. */
    const dpf::DpfParameters dpf_params;   /**< The parameters for DPF. */
    const bool               debug;        /**< Toggle this flag to enable/disable debugging. */
    const DebugInfo          dbg_info;     /**< Debug information. */

    /**
     * @brief Default constructor for FssRankParameters.
     */
    FssRankParameters();

    /**
     * @brief Parameterized constructor for FssRankParameters.
     * @param t The size of the text in bits.
     * @param debug Debug utils::Mode flag.
     */
    FssRankParameters(const uint32_t t, const DebugInfo &dbg_info);
};

struct FssRankKey {
    dpf::DpfKey dpf_key; /**< The DPF key associated with the FssRankKey. */
    uint32_t    shr_in;  /**< Random value for input. */

    /**
     * @brief Default constructor for FssRankKey.
     */
    FssRankKey();

    /**
     * @brief Copy constructor (deleted).
     */
    FssRankKey(const FssRankKey &) = delete;

    /**
     * @brief Copy assignment operator (deleted).
     */
    FssRankKey &operator=(const FssRankKey &) = delete;

    /**
     * @brief Move constructor (default).
     */
    FssRankKey(FssRankKey &&) noexcept = default;

    /**
     * @brief Move assignment operator (default).
     */
    FssRankKey &operator=(FssRankKey &&) noexcept = default;

    bool operator==(const FssRankKey &rhs) const {
        return this->dpf_key == rhs.dpf_key && this->shr_in == rhs.shr_in;
    }

    bool operator!=(const FssRankKey &rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Pruint32_t the details of the rank key.
     * @param debug Debug utils::Mode flag.
     */
    void PrintFssRankKey(const FssRankParameters &params, const bool debug) const;

    /**
     * @brief Free the resources associated with the rank key.
     */
    void FreeFssRankKey();
};

/**
 * @class FssRank
 * @brief A class providing functionalities for rank calculations.
 */
class FssRank {
public:
    /**
     * @brief Parameterized constructor for FssRank.
     * @param params The parameters for DpfSelect.
     */
    FssRank(const FssRankParameters params);

    /**
     * @brief Generate keys for FssRank keys.
     * @return A pair of FssRankKey.
     */
    std::pair<FssRankKey, FssRankKey> GenerateKeys() const;

    /**
     * @brief Evaluate rank for a given sentence and position.
     * @param rank_key Rank key.
     * @param sentence The sentence to be evaluated.
     * @param pos The position to evaluate the rank at.
     * @return An array of two uint32_t values representing the rank calculation result.
     */
    std::array<uint32_t, 2> Evaluate(const FssRankKey &rank_key, const std::string &sentence, const uint32_t pos) const;

private:
    const FssRankParameters             params_; /**< The parameters for FssRank. */
    const dpf::DistributedPointFunction dpf_;    /**< The DPF object for FssRank. */
};

namespace test {

void Test_FssRank(tools::secret_sharing::Party &party, TestInfo &test_info);

}    // namespace test

}    // namespace rank
}    // namespace fss

#endif    // RANK_FSS_RANK_H_
