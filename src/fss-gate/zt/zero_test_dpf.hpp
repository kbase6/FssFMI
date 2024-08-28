/**
 * @file zero_test_dpf.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-19
 * @copyright Copyright (c) 2024
 * @brief Zero Test class.
 */

#ifndef ZT_ZERO_TEST_DPF_H_
#define ZT_ZERO_TEST_DPF_H_

#include "../../fss-base/dpf/distributed_point_function.hpp"
#include "../../tools/secret_sharing.hpp"

namespace fss {
namespace zt {

/**
 * @struct ZeroTestParameters
 * @brief A struct to hold params for the Zero Test technique.
 */
struct ZeroTestParameters {
    const uint32_t           input_bitsize;   /**< The size of input in bits. */
    const uint32_t           element_bitsize; /**< The size of each element in bits. */
    const dpf::DpfParameters dpf_params;      /**< DPF parameters for ZeroTest. */
    const bool               debug;           /**< Debug utils::Mode flag. */
    const DebugInfo          dbg_info;        /**< Debug information. */

    /**
     * @brief Default constructor for ZeroTestParameters.
     */
    ZeroTestParameters();

    /**
     * @brief Parameterized constructor for ZeroTestParameters.
     * @param n The input bitsize.
     * @param e The element bitsize.
     * @param debug Debug utils::Mode flag.
     */
    ZeroTestParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info);
};

/**
 * @struct ZeroTestKey
 * @brief A struct representing a key for the Zero Test technique.
 */
struct ZeroTestKey {
    dpf::DpfKey dpf_key; /**< The DPF key associated with the ZeroTestKey. */
    uint32_t    shr_in;  /**< Input value for shift-right operation. */

    /**
     * @brief Default constructor for ZeroTestKey.
     */
    ZeroTestKey();

    /**
     * @brief Copy constructor is deleted to prevent copying of ZeroTestKey.
     */
    ZeroTestKey(const ZeroTestKey &) = delete;

    /**
     * @brief Copy assignment operator is deleted to prevent copying of ZeroTestKey.
     */
    ZeroTestKey &operator=(const ZeroTestKey &) = delete;

    /**
     * @brief Move constructor for ZeroTestKey.
     */
    ZeroTestKey(ZeroTestKey &&) noexcept = default;

    /**
     * @brief Move assignment operator for ZeroTestKey.
     */
    ZeroTestKey &operator=(ZeroTestKey &&) noexcept = default;

    bool operator==(const ZeroTestKey &rhs) const {
        return this->dpf_key == rhs.dpf_key && this->shr_in == rhs.shr_in;
    }

    bool operator!=(const ZeroTestKey &rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Print the details of the ZeroTestKey.
     * @param debug Debug utils::Mode flag.
     */
    void PrintZeroTestKey(const ZeroTestParameters &params, const bool debug) const;

    /**
     * @brief Free the resources associated with the ZeroTestKey.
     */
    void FreeZeroTestKey();
};

/**
 * @class ZeroTest
 * @brief A class implementing the Zero Test technique.
 */
class ZeroTest {
public:
    /**
     * @brief Constructor for ZeroTest.
     * @param params ZeroTestParameters instance containing the params for ZeroTest.
     */
    ZeroTest(const ZeroTestParameters params);

    /**
     * @brief Generate a pair of ZeroTestKey instances.
     * @return A pair of ZeroTestKey instances representing the generated keys.
     */
    std::pair<ZeroTestKey, ZeroTestKey> GenerateKeys() const;

    /**
     * @brief Evaluate the Zero Test at a specific point.
     * @param zt_key The ZeroTestKey instance to use for evaluation.
     * @param x The input value for evaluation.
     * @return The result of the evaluation.
     */
    uint32_t EvaluateAt(const ZeroTestKey &zt_key, const uint32_t x) const;

private:
    const ZeroTestParameters            params_; /**< Parameters for ZeroTest. */
    const dpf::DistributedPointFunction dpf_;    /**< Underlying DistributedPointFunction instance. */
};

namespace test {

void Test_ZeroTest(tools::secret_sharing::Party &party, TestInfo &test_info);

}    // namespace test

}    // namespace zt
}    // namespace fss

#endif    // ZT_ZERO_TEST_DPF_H_
