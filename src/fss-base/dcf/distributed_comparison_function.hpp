/**
 * @file distributed_comparison_function.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-02-05
 * @copyright Copyright (c) 2024
 * @brief Distributed Comparison Function (DCF) class.
 */

#ifndef DCF_DISTRIBUTED_COMPARISON_FUNCTION_H_
#define DCF_DISTRIBUTED_COMPARISON_FUNCTION_H_

#include <vector>

#include "../fss_block.hpp"
#include "../fss_configure.hpp"

namespace fss {
namespace dcf {

/**
 * @struct DcfParameters
 * @brief A struct to hold params for the Distributed Comparison Function (DCF).
 */
struct DcfParameters {
    const uint32_t input_bitsize;   /**< The size of input in bits. */
    const uint32_t element_bitsize; /**< The size of each element in bits. */
    const bool     debug;           /**< Toggle this flag to enable/disable debugging. */

    /**
     * @brief Default constructor for DcfParameters.
     */
    DcfParameters();

    /**
     * @brief Parameterized constructor for DcfParameters.
     * @param n The input bitsize.
     * @param e The element bitsize.
     * @param debug Toggle this flag to enable/disable debugging.
     */
    DcfParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info);
};

/**
 * @struct CorrectionWord
 * @brief A struct representing a correction word for error correction.
 */
struct CorrectionWord {
    Block    seed;          /**< Seed for correction. */
    bool     control_left;  /**< Left control bit for correction. */
    bool     control_right; /**< Right control bit for correction. */
    uint32_t value;         /**< Value associated with the correction. */

    /**
     * @brief Default constructor for CorrectionWord.
     */
    CorrectionWord();

    /**
     * @brief Copy constructor is default for CorrectionWord.
     */
    CorrectionWord(const CorrectionWord &) = default;

    /**
     * @brief Copy assignment operator is default for CorrectionWord.
     */
    CorrectionWord &operator=(const CorrectionWord &) = default;
};

/**
 * @struct DcfKey
 * @brief A struct representing a key for the Distributed Comparison Function (DCF).
 */
struct DcfKey {
    uint32_t        party_id;         /**< The ID of the party associated with the key. */
    Block           init_seed;        /**< Seed for the DCF key. */
    uint32_t        cw_length;        /**< Size of Correction words. */
    CorrectionWord *correction_words; /**< Pointer to an array of CorrectionWord instances. */
    uint32_t        output;           /**< Output value associated with the key. */

    /**
     * @brief Default constructor for DcfKey.
     */
    DcfKey(){};

    /**
     * @brief Copy constructor is deleted to prevent copying of DcfKey.
     */
    DcfKey(const DcfKey &) = delete;

    /**
     * @brief Copy assignment operator is deleted to prevent copying of DcfKey.
     */
    DcfKey &operator=(const DcfKey &) = delete;

    /**
     * @brief Move constructor for DcfKey.
     */
    DcfKey(DcfKey &&) noexcept = default;

    /**
     * @brief Move assignment operator for DcfKey.
     */
    DcfKey &operator=(DcfKey &&) noexcept = default;

    bool operator==(const DcfKey &rhs) const {
        bool result = (party_id == rhs.party_id) && (init_seed == rhs.init_seed) && (cw_length == rhs.cw_length);
        for (uint32_t i = 0; i < cw_length; i++) {
            result &= (correction_words[i].seed == rhs.correction_words[i].seed) && (correction_words[i].control_left == rhs.correction_words[i].control_left) && (correction_words[i].control_right == rhs.correction_words[i].control_right);
        }
        result &= (output == rhs.output);
        return result;
    }

    bool operator!=(const DcfKey &rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Initialize the DcfKey with specified values.
     * @param n The size of input in bits.
     * @param party_id The ID of the party associated with the key.
     */
    void Initialize(const uint32_t n, const uint32_t party_id);

    /**
     * @brief Print the details of the DcfKey.
     * @param debug Toggle this flag to enable/disable debugging.
     */
    void PrintDcfKey(const bool debug) const;

    /**
     * @brief Free the resources associated with the DcfKey.
     */
    void FreeDcfKey();
};

/**
 * @brief Class representing the Distributed Comparison Function (DCF).
 */
class DistributedComparisonFunction {
public:
    /**
     * @brief Constructor for DistributedComparisonFunction.
     * @param params Parameters for DCF.
     */
    DistributedComparisonFunction(const DcfParameters params);

    /**
     * @brief Generate a pair of DcfKey instances for given alpha and beta values.
     * @param alpha The alpha value for DCF key generation.
     * @param beta The beta value for DCF key generation.
     * @return A pair of DcfKey.
     */
    std::pair<DcfKey, DcfKey> GenerateKeys(const uint32_t alpha, const uint32_t beta) const;

    /**
     * @brief Evaluate the DCF at a specific point 'x' using the provided key.
     * @param key The DCF key to use for evaluation.
     * @param x The point 'x' at which to evaluate the DCF.
     * @return The evaluation result at point 'x'.
     */
    uint32_t EvaluateAt(const DcfKey &key, const uint32_t x) const;

private:
    const DcfParameters params_; /**< Parameters for the DistributedComparisonFunction. */

    /**
     * @brief Evaluate the next seed in the Distributed Comparison Function (DCF) tree.
     *
     * This method is used internally to evaluate the next seed in the DCF tree during
     * the evaluation process. It updates the expanded seeds, values, and control bits
     * based on the current tree level, correction word, seed, and control bit.
     *
     * @param current_tree_level The current tree level being evaluated.
     * @param correction_word The correction word for the current node.
     * @param current_seed The current seed value.
     * @param current_control_bit The current control bit.
     * @param expanded_seeds An array of Block values used to store expanded seeds.
     * @param expanded_values An array of Block values used to store expanded values.
     * @param expanded_control_bits An array of boolean values used to store expanded control bits.
     */
    void EvaluateNextSeed(
        const uint32_t current_tree_level, const CorrectionWord &correction_word,
        const Block &current_seed, const bool current_control_bit,
        std::array<Block, 2> &expanded_seeds, std::array<Block, 2> &expanded_values, std::array<bool, 2> &expanded_control_bits) const;
};

namespace test {

void Test_Dcf(TestInfo &test_info);

}    // namespace test

}    // namespace dcf
}    // namespace fss

#endif    // DCF_DISTRIBUTED_COMPARISON_FUNCTION_H_
