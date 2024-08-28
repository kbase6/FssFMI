/**
 * @file integer_comparison.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-02-05
 * @copyright Copyright (c) 2024
 * @brief Integer Comparison class.
 */

#ifndef COMP_INTEGER_COMPARISON_H_
#define COMP_INTEGER_COMPARISON_H_

#include "../../fss-base/ddcf/dual_dcf.hpp"
#include "../../tools/secret_sharing.hpp"

namespace fss {
namespace comp {

/**
 * @struct CompParameters
 * @brief A struct to hold params for the CompIntegerComparison.
 */
struct CompParameters {
    const uint32_t  input_bitsize;   /**< The size of input in bits. */
    const uint32_t  element_bitsize; /**< The size of each element in bits. */
    const bool      debug;           /**< Toggle this flag to enable/disable debugging. */
    const DebugInfo dbg_info;        /**< Debug information. */

    /**
     * @brief Default constructor for CompParameters.
     */
    CompParameters();

    /**
     * @brief Parameterized constructor for CompParameters.
     * @param n The input bitsize.
     * @param e The element bitsize.
     * @param debug Debug utils::Mode flag.
     */
    CompParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info);
};

/**
 * @struct CompKey
 * @brief A struct representing a key for integer comparison.
 */
struct CompKey {
    ddcf::DdcfKey ddcf_key; /**< The DDCf key associated with the CompKey. */
    uint32_t      shr1_in;  /**< Input value for shift-right operation 1. */
    uint32_t      shr2_in;  /**< Input value for shift-right operation 2. */
    uint32_t      shr_out;  /**< Output value for shift-right operation. */

    /**
     * @brief Default constructor for CompKey.
     */
    CompKey();

    /**
     * @brief Copy constructor is deleted to prevent copying of CompKey.
     */
    CompKey(const CompKey &) = delete;

    /**
     * @brief Copy assignment operator is deleted to prevent copying of CompKey.
     */
    CompKey &operator=(const CompKey &) = delete;

    /**
     * @brief Move constructor for CompKey.
     */
    CompKey(CompKey &&) noexcept = default;

    /**
     * @brief Move assignment operator for CompKey.
     */
    CompKey &operator=(CompKey &&) noexcept = default;

    bool operator==(const CompKey &rhs) const {
        return this->ddcf_key == rhs.ddcf_key && this->shr1_in == rhs.shr1_in && this->shr2_in == rhs.shr2_in && this->shr_out == rhs.shr_out;
    }

    bool operator!=(const CompKey &rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Print the details of the CompKey.
     * @param debug Debug utils::Mode flag.
     */
    void PrintCompKey(const bool debug) const;

    /**
     * @brief Free the resources associated with the CompKey.
     */
    void FreeCompKey();
};

/**
 * @class IntegerComparison
 * @brief A class implementing integer comparison using the Dual Distributed Comparison Function (DDCF) technique.
 */
class IntegerComparison {
public:
    /**
     * @brief Constructor for IntegerComparison.
     * @param params CompParameters instance containing the params for IntegerComparison.
     */
    IntegerComparison(CompParameters params);

    /**
     * @brief Generate a pair of CompKey instances based on input params.
     * @return A pair of CompKey instances representing the generated keys.
     */
    std::pair<CompKey, CompKey> GenerateKeys() const;

    /**
     * @brief Evaluate integer comparison using the provided CompKey.
     *
     * This method evaluates integer comparison using the provided CompKey instance.
     * It takes two input values, 'x' and 'y', and returns the result of the comparison.
     *
     * @param comp_key The CompKey instance to use for evaluation.
     * @param x The first input value for comparison.
     * @param y The second input value for comparison.
     * @return The result of the comparison.
     */
    uint32_t Evaluate(const CompKey &comp_key, const uint32_t x, const uint32_t y) const;

private:
    const CompParameters                          params_; /**< Parameters for IntegerComparison. */
    const ddcf::DualDistributedComparisonFunction ddcf_;   /**< Underlying DualDistributedComparisonFunction instance. */
};

namespace test {

void Test_Comp(tools::secret_sharing::Party &party, const TestInfo &test_info);

}    // namespace test

}    // namespace comp
}    // namespace fss

#endif    // COMP_INTEGER_COMPARISON_H_
