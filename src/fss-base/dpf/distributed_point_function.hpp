/**
 * @file distributed_point_function.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-10
 * @copyright Copyright (c) 2024
 * @brief Distributed Point Function (DPF) class.
 */

#ifndef DPF_DISTRIBUTED_POINT_FUNCTION_H_
#define DPF_DISTRIBUTED_POINT_FUNCTION_H_

#include <vector>

#include "../fss_block.hpp"
#include "../fss_configure.hpp"

namespace fss {
namespace dpf {

/**
 * @struct DpfParameters
 * @brief A struct to hold params for the Distributed Point Function (DPF).
 */
struct DpfParameters {
    const uint32_t input_bitsize;     /**< The size of input in bits. */
    const uint32_t element_bitsize;   /**< The size of each element in bits. */
    const uint32_t terminate_bitsize; /**< The size of the termination bits. */
    const bool     debug;             /**< Toggle this flag to enable/disable debugging. */

    /**
     * @brief Default constructor for DpfParameters.
     */
    DpfParameters();

    /**
     * @brief Parameterized constructor for DpfParameters.
     * @param n The input bitsize.
     * @param e The element bitsize.
     * @param debug Toggle this flag to enable/disable debugging.
     */
    DpfParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info);

    /**
     * @brief Compute the number of levels to terminate the DPF evaluation.
     *
     * @return The number of levels to terminate the DPF evaluation.
     */
    uint32_t ComputeTerminateLevel();
};

/**
 * @struct CorrectionWord
 * @brief A struct representing a correction word ([seed, control left, control right]).
 */
struct CorrectionWord {
    Block seed;          /**< Seed for correction. */
    bool  control_left;  /**< Left control bit for correction. */
    bool  control_right; /**< Right control bit for correction. */

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
 * @struct DpfKey
 * @brief A struct representing a key for the Distributed Point Function (DPF).
 * @warning `DpfKey` must call initialize before use.
 */
struct DpfKey {
    uint32_t        party_id;         /**< The ID of the party associated with the key. */
    Block           init_seed;        /**< Seed for the DPF key. */
    uint32_t        cw_length;        /**< Size of Correction words. */
    CorrectionWord *correction_words; /**< Pointer to an array of CorrectionWord instances. */
    Block           output;           /**< Output of the DPF key. */

    /**
     * @brief Default constructor for DpfKey.
     */
    DpfKey(){};

    /**
     * @brief Copy constructor is deleted to prevent copying of DpfKey.
     */
    DpfKey(const DpfKey &) = delete;

    /**
     * @brief Copy assignment operator is deleted to prevent copying of DpfKey.
     */
    DpfKey &operator=(const DpfKey &) = delete;

    /**
     * @brief Move constructor for DpfKey.
     */
    DpfKey(DpfKey &&) noexcept = default;

    /**
     * @brief Move assignment operator for DpfKey.
     */
    DpfKey &operator=(DpfKey &&) noexcept = default;

    bool operator==(const DpfKey &rhs) const {
        bool result = (party_id == rhs.party_id) && (init_seed == rhs.init_seed) && (cw_length == rhs.cw_length);
        for (uint32_t i = 0; i < cw_length; i++) {
            result &= (correction_words[i].seed == rhs.correction_words[i].seed) && (correction_words[i].control_left == rhs.correction_words[i].control_left) && (correction_words[i].control_right == rhs.correction_words[i].control_right);
        }
        result &= (output == rhs.output);
        return result;
    }

    bool operator!=(const DpfKey &rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Initialize the DpfKey with specified values.
     * @param params DpfParameters for the DpfKey.
     * @param party_id The ID of the party associated with the key.
     * @param is_naive Toggle this flag to enable/disable naive evaluation.
     */
    void Initialize(const DpfParameters &params, const uint32_t party_id, const bool is_naive = false);

    /**
     * @brief Print the details of the DpfKey.
     * @param n The size of input in bits.
     * @param params DpfParameters for the DpfKey.
     * @param debug Toggle this flag to enable/disable debugging.
     */
    void PrintDpfKey(const DpfParameters &params, const bool debug, const bool is_naive = false) const;

    /**
     * @brief Free the resources associated with the DpfKey.
     */
    void FreeDpfKey();
};

/**
 * @class DistributedPointFunction
 * @brief A class representing a Distributed Point Function (DPF).
 */
class DistributedPointFunction {
public:
    /**
     * @brief Constructor for DistributedPointFunction.
     * @param params Parameters for DPF.
     */
    DistributedPointFunction(const DpfParameters params);

    /**
     * @brief Generate a pair of DpfKey instances for given alpha and beta values with early termination.
     * @param alpha The alpha value for DPF key generation.
     * @param beta The beta value for DPF key generation.
     * @return A pair of DpfKey.
     */
    std::pair<DpfKey, DpfKey> GenerateKeys(const uint32_t alpha, const uint32_t beta) const;

    /**
     * @brief Evaluate the DPF at the given key and input with early termination.
     * @param key The DpfKey instance to use for evaluation.
     * @param x The input value for evaluation.
     * @return The result of the evaluation.
     */
    uint32_t EvaluateAt(const DpfKey &key, const uint32_t x) const;

    /**
     * @brief Evaluate the Distributed Point Function (DPF) over the full domain.
     *
     * This method evaluates the DPF over its entire domain using the provided key. By using full domain evaluation and early termination O(N).
     * It returns a vector of uint32_t values representing the results of the evaluation
     * for each possible input value in the full domain.
     *
     * @param key The DpfKey instance to use for evaluation.
     * @param outputs A vector of uint32_t values representing the evaluation results over the full domain.
     */
    void EvaluateFullDomain(const DpfKey &key, std::vector<uint32_t> &outputs) const;

    /**
     * @brief Evaluate the Distributed Point Function (DPF) over the full domain.
     *
     * This method evaluates the DPF over its entire domain using the provided key. By using full domain evaluation and early termination O(N).
     * It returns a vector of uint32_t values representing the results of the evaluation
     * for each possible input value in the full domain.
     *
     * @param key The DpfKey instance to use for evaluation.
     * @param outputs A vector of uint32_t values representing the evaluation results over the full domain.
     */
    void EvaluateFullDomainOneBit(const DpfKey &key, std::vector<uint32_t> &outputs) const;

    /**
     * @brief Evaluate the Distributed Point Function (DPF) over the full domain in a non-recursive manner with early termination.
     *
     * @param key The DpfKey instance to use for evaluation.
     * @param outputs A vector of uint32_t values representing the evaluation results over the full domain.
     */
    void FullDomainNonRecursive(const DpfKey &key, std::vector<uint32_t> &outputs) const;

    /**
     * @brief Evaluate the Distributed Point Function (DPF) over the full domain in a non-recursive manner with early termination.
     *
     * @param key The DpfKey instance to use for evaluation.
     * @param outputs A vector of uint32_t values representing the evaluation results over the full domain.
     */
    void FullDomainNonRecursiveParallel_4(const DpfKey &key, std::vector<uint32_t> &outputs) const;

    /**
     * @brief Evaluate the Distributed Point Function (DPF) over the full domain in a non-recursive manner with early termination.
     *
     * @param key The DpfKey instance to use for evaluation.
     * @param outputs A vector of uint32_t values representing the evaluation results over the full domain.
     */
    void FullDomainNonRecursiveParallel_8(const DpfKey &key, std::vector<uint32_t> &outputs) const;

    /**
     * @brief Evaluate the Distributed Point Function (DPF) over the full domain in a non-recursive manner with early termination.
     *
     * @param key The DpfKey instance to use for evaluation.
     * @param outputs A vector of uint32_t values representing the evaluation results over the full domain.
     */
    void FullDomainNonRecursiveParallel_128(const DpfKey &key, std::vector<uint32_t> &outputs) const;

    /**
     * @brief Evaluate the Distributed Point Function (DPF) over the full domain in a recursive manner *with* early termination.
     *
     * @param key The DpfKey instance to use for evaluation.
     * @param outputs A vector of uint32_t values representing the evaluation results over the full domain.
     *
     * @warning Slow using recursive evaluation.
     */
    void FullDomainRecursive(const DpfKey &key, std::vector<uint32_t> &outputs) const;

    /**
     * @brief Generate a pair of DpfKey instances for given alpha and beta values without early termination.
     * @param alpha
     * @param beta
     * @return std::pair<DpfKey, DpfKey>
     */
    std::pair<DpfKey, DpfKey> GenerateKeysNaive(const uint32_t alpha, const uint32_t beta) const;

    /**
     * @brief Evaluate the DPF at the given key and input withour early termination.
     * @param key The DpfKey instance to use for evaluation.
     * @param x The input value for evaluation.
     * @return The result of the evaluation.
     */
    uint32_t EvaluateAtNaive(const DpfKey &key, const uint32_t x) const;

    /**
     * @brief Evaluate the Distributed Point Function (DPF) over the full domain in a naive manner.
     *
     * This method evaluates the DPF over its entire domain using EvaluateAt() for each input point O(N logN).
     * It returns a vector of uint32_t values representing the results of the evaluation
     * for each possible input value in the full domain.
     *
     * @param key The DpfKey instance to use for evaluation. (Naive version)
     * @param outputs A vector of uint32_t values representing the evaluation results over the full domain.
     */
    void FullDomainNaiveNaive(const DpfKey &key, std::vector<uint32_t> &outputs) const;

private:
    const DpfParameters params_; /**< Parameters for DistributedPointFunction. */

    /**
     * @brief Generates the next seed for the distributed point function.
     *
     * @param current_tree_level The current tree level.
     * @param current_bit The current bit.
     * @param keys The array of DpfKey objects.
     * @param current_seeds The array of current seeds.
     * @param current_control_bits The array of current control bits.
     */
    void GenerateNextSeed(
        const uint32_t current_tree_level, const bool current_bit,
        const std::array<DpfKey, 2> &keys, std::array<Block, 2> &current_seeds, std::array<bool, 2> &current_control_bits) const;

    /**
     * @brief Evaluates the next seed for the distributed point function.
     *
     * This function expands the given current seed using pseudorandom generators and applies a correction word if the current control bit is set.
     *
     * @param current_tree_level The current tree level.
     * @param correction_word The correction word to apply if the current control bit is set.
     * @param current_seed The current seed to evaluate.
     * @param current_control_bit The current control bit.
     * @param expanded_seeds The expanded seeds after evaluation.
     * @param expanded_control_bits The expanded control bits after evaluation.
     */
    void EvaluateNextSeed(
        const uint32_t current_tree_level, const CorrectionWord &correction_word,
        const Block &current_seed, const bool current_control_bit,
        std::array<Block, 2> &expanded_seeds, std::array<bool, 2> &expanded_control_bits) const;

    /**
     * @brief Traverses the distributed point function.
     *
     * This function recursively traverses the distributed point function based on the given params.
     *
     * @param current_seed The current seed block.
     * @param current_control_bit The current control bit.
     * @param key The DpfKey object.
     * @param i The current value of i.
     * @param j The current value of j.
     * @param outputs The vector to store the outputs.
     */
    void Traverse(const Block &current_seed, const bool current_control_bit, const DpfKey &key, uint32_t i, uint32_t j, std::vector<uint32_t> &output) const;

    /**
     * @brief Set the output of the DPF key based on the input alpha, beta, and control bit.
     *
     * @param alpha The input alpha value.
     * @param beta The input beta value.
     * @param control_bit The control bit value.
     * @param seeds The seeds of the DPF key.
     * @param keys The DPF keys to set the output.
     */
    void SetKeyOutput(const uint32_t alpha, const uint32_t beta, const bool control_bit, const std::array<Block, 2> &seeds, std::array<DpfKey, 2> &keys) const;

    /**
     * @brief  Compute the output block based on the current seed, control bit, and DPF key.
     *
     * @param current_seed The current seed block.
     * @param current_control_bit The current control bit.
     * @param key The DPF key.
     * @return The output block.
     */
    Block ComputeOutputBlock(const Block &current_seed, const bool current_control_bit, const DpfKey &key) const;
};

namespace test {

void Test_Dpf(TestInfo &test_info);

}    // namespace test

namespace bench {

void Bench_Dpf(const BenchInfo &bench_info);

}    // namespace bench

}    // namespace dpf
}    // namespace fss

#endif    // DPF_DISTRIBUTED_POINT_FUNCTION_H_
