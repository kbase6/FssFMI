/**
 * @file dual_dcf.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-02-05
 * @copyright Copyright (c) 2024
 * @brief Dual DCF (DDCF) class.
 */

#ifndef DDCF_DUAL_DCF_H_
#define DDCF_DUAL_DCF_H_

#include "../dcf/distributed_comparison_function.hpp"

namespace fss {
namespace ddcf {

/**
 * @struct DdcfParameters
 * @brief A struct to hold params for the Dual Distributed Comparison Function (DDCF).
 */
struct DdcfParameters {
    const uint32_t  input_bitsize;   /**< The size of input in bits. */
    const uint32_t  element_bitsize; /**< The size of each element in bits. */
    const bool      debug;           /**< Toggle this flag to enable/disable debugging. */
    const DebugInfo dbg_info;        /**< Debug information. */

    /**
     * @brief Default constructor for DdcfParameters.
     */
    DdcfParameters();

    /**
     * @brief Parameterized constructor for DdcfParameters.
     * @param n The input bitsize.
     * @param e The element bitsize.
     * @param debug Toggle this flag to enable/disable debugging.
     */
    DdcfParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info);
};

/**
 * @struct DdcfKey
 * @brief A struct representing a Dual Distributed Comparison Function (DDCF) key.
 */
struct DdcfKey {
    dcf::DcfKey dcf_key; /**< The DCF key associated with the DDCF key. */
    uint32_t    mask;    /**< The mask value for the DDCF key. */

    /**
     * @brief Default constructor for DdcfKey.
     */
    DdcfKey(){};

    /**
     * @brief Copy constructor is deleted to prevent copying of DdcfKey.
     */
    DdcfKey(const DdcfKey &) = delete;

    /**
     * @brief Copy assignment operator is deleted to prevent copying of DdcfKey.
     */
    DdcfKey &operator=(const DdcfKey &) = delete;

    /**
     * @brief Move constructor for DdcfKey.
     */
    DdcfKey(DdcfKey &&) noexcept = default;

    /**
     * @brief Move assignment operator for DdcfKey.
     */
    DdcfKey &operator=(DdcfKey &&) noexcept = default;

    bool operator==(const DdcfKey &rhs) const {
        return dcf_key == rhs.dcf_key && mask == rhs.mask;
    }

    bool operator!=(const DdcfKey &rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Print the details of the DdcfKey.
     * @param debug Toggle this flag to enable/disable debugging.
     */
    void PrintDdcfKey(const bool debug) const;

    /**
     * @brief Free the resources associated with the DdcfKey.
     */
    void FreeDdcfKey();
};

/**
 * @class DualDistributedComparisonFunction
 * @brief A class implementing the Dual Distributed Comparison Function (DDCF) technique.
 */
class DualDistributedComparisonFunction {
public:
    /**
     * @brief Constructor for DualDistributedComparisonFunction.
     * @param params DdcfParameters instance containing the params for DDCF.
     */
    DualDistributedComparisonFunction(const DdcfParameters params);

    /**
     * @brief Generate a pair of DdcfKey instances based on input params.
     * @param alpha The alpha value for key generation.
     * @param beta_1 The beta_1 value for key generation.
     * @param beta_2 The beta_2 value for key generation.
     * @return A pair of DdcfKey instances representing the generated keys.
     */
    std::pair<DdcfKey, DdcfKey> GenerateKeys(const uint32_t alpha, const uint32_t beta_1, const uint32_t beta_2) const;

    /**
     * @brief Evaluate the Dual Distributed Comparison Function at a specific point.
     * @param ddcf_key The DdcfKey instance to use for evaluation.
     * @param x The input value for evaluation.
     * @return The result of the evaluation.
     */
    uint32_t EvaluateAt(const DdcfKey &ddcf_key, uint32_t x) const;

private:
    const DdcfParameters                     params_; /**< Parameters for DDCF. */
    const dcf::DistributedComparisonFunction dcf_;    /**< Underlying DistributedComparisonFunction instance. */
};

namespace test {

void Test_Ddcf(TestInfo &test_info);

}    // namespace test

}    // namespace ddcf
}    // namespace fss

#endif    // DDCF_DUAL_DCF_H_
