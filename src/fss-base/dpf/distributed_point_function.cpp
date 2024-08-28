/**
 * @file distributed_point_function.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-10
 * @copyright Copyright (c) 2024
 * @brief Distributed Point Function (DPF) implementation.
 */

#include "distributed_point_function.hpp"

#include <algorithm>

#include "../../utils/logger.hpp"
#include "../../utils/timer.hpp"
#include "../../utils/utils.hpp"
#include "../prg/prg.hpp"

namespace {

// Pseudorandom generators for various key values
static const fss::prg::PRG prg_seed_left  = fss::prg::PRG::Create(fss::kPrgKeySeedLeft);
static const fss::prg::PRG prg_seed_right = fss::prg::PRG::Create(fss::kPrgKeySeedRight);

}    // namespace

namespace fss {
namespace dpf {

DpfParameters::DpfParameters()
    : input_bitsize(0), element_bitsize(0), terminate_bitsize(0), debug(false) {
}

DpfParameters::DpfParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info)
    : input_bitsize(n), element_bitsize(e), terminate_bitsize(ComputeTerminateLevel()), debug(dbg_info.dpf_debug) {
}

uint32_t DpfParameters::ComputeTerminateLevel() {
    int32_t nu = static_cast<int32_t>(std::min(std::ceil(input_bitsize - std::log2(fss::kSecurityParameter / element_bitsize)), static_cast<double>(input_bitsize)));
    return (nu < 0) ? 0 : static_cast<uint32_t>(nu);
}

void DpfKey::Initialize(const DpfParameters &params, const uint32_t party_id, const bool is_naive) {
    this->party_id         = party_id;
    this->init_seed        = Block(zero_block);
    this->cw_length        = is_naive ? params.input_bitsize : params.terminate_bitsize;
    this->correction_words = new CorrectionWord[cw_length];
    this->output           = Block(zero_block);
}

void DpfKey::PrintDpfKey(const DpfParameters &params, const bool debug, const bool is_naive) const {
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("DPF Key"), debug);
    utils::Logger::TraceLog(LOCATION, "Party ID: " + std::to_string(this->party_id), debug);
    this->init_seed.PrintBlockHexTrace(LOCATION, "Initial seed: ", debug);
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Correction words"), debug);
    for (uint32_t i = 0; i < this->cw_length; i++) {
        this->correction_words[i].seed.PrintBlockHexTrace(LOCATION, "Level(" + std::to_string(i) + ") Seed -> ", debug);
        utils::Logger::TraceLog(LOCATION, "Level(" + std::to_string(i) + ") Control bit (L):" + std::to_string(this->correction_words[i].control_left) + ", (R): " + std::to_string(this->correction_words[i].control_right), debug);
    }
    if (is_naive) {
        utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(output.Convert(params.element_bitsize)), debug);
    } else {
        utils::Logger::TraceLog(LOCATION, "Output: " + utils::VectorToStr(output.ConvertVec(utils::Pow(2, params.input_bitsize - params.terminate_bitsize), params.element_bitsize)), debug);
    }
    utils::Logger::TraceLog(LOCATION, utils::kDash, debug);
#endif
}

void DpfKey::FreeDpfKey() {
    delete[] this->correction_words;
}

CorrectionWord::CorrectionWord()
    : seed(Block(zero_block)), control_left(false), control_right(false) {
}

DistributedPointFunction::DistributedPointFunction(const DpfParameters params)
    : params_(params) {
}

std::pair<DpfKey, DpfKey> DistributedPointFunction::GenerateKeys(const uint32_t alpha, const uint32_t beta) const {
    uint32_t n  = this->params_.input_bitsize;
    uint32_t nu = this->params_.terminate_bitsize;
#ifdef LOG_LEVEL_TRACE
    uint32_t e     = this->params_.element_bitsize;
    bool     debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Generate DPF keys"), debug);
    utils::Logger::TraceLog(LOCATION, "(input size, element size) = (" + std::to_string(n) + ", " + std::to_string(e) + ")", debug);
    utils::Logger::TraceLog(LOCATION, "terminate size = " + std::to_string(nu), debug);
    utils::Logger::TraceLog(LOCATION, "(alpha, beta) = (" + std::to_string(alpha) + ", " + std::to_string(beta) + ")", debug);
#endif

    // Create DPF keys for both parties and initialize them.
    std::array<DpfKey, 2> keys;    // keys[party id]
    keys[0].Initialize(this->params_, 0);
    keys[1].Initialize(this->params_, 1);

    // Set initial seeds and control bit
    std::array<Block, 2> seeds;                 // seeds[party id]
    std::array<bool, 2>  control_bits{0, 1};    // control_bits[party id]
    // Set initial seeds for both parties.
    seeds[0].SetRandom();
    seeds[1].SetRandom();
    keys[0].init_seed = seeds[0];
    keys[1].init_seed = seeds[1];

#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Set initial seed and control bit", debug);
    seeds[0].PrintBlockHexTrace(LOCATION, "ID=" + std::to_string(0) + " Initial seed: ", debug);
    seeds[1].PrintBlockHexTrace(LOCATION, "ID=" + std::to_string(1) + " Initial seed: ", debug);
    utils::Logger::TraceLog(LOCATION, "ID=" + std::to_string(0) + " Control bit: " + std::to_string(control_bits[0]), debug);
    utils::Logger::TraceLog(LOCATION, "ID=" + std::to_string(1) + " Control bit: " + std::to_string(control_bits[1]), debug);
#endif

    // line 5-13: Generate next seed and compute correction words
    for (uint32_t i = 0; i < nu; i++) {
        bool current_bit = (alpha & (1 << (n - i - 1))) != 0;
        GenerateNextSeed(i, current_bit, keys, seeds, control_bits);
    }
    // Calculate and set the output for both keys.
    SetKeyOutput(alpha, beta, control_bits[1], seeds, keys);

// Log the generated keys and output.
#ifdef LOG_LEVEL_TRACE
    utils::AddNewLine(debug);
    keys[0].PrintDpfKey(params_, debug);
    utils::AddNewLine(debug);
    keys[1].PrintDpfKey(params_, debug);
    utils::AddNewLine(debug);
#endif

    // Return the generated keys as a pair.
    return std::make_pair(std::move(keys[0]), std::move(keys[1]));
}

uint32_t DistributedPointFunction::EvaluateAt(const DpfKey &key, const uint32_t x) const {
    uint32_t n  = this->params_.input_bitsize;
    uint32_t e  = this->params_.element_bitsize;
    uint32_t nu = this->params_.terminate_bitsize;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate input with DPF key"), debug);
    utils::Logger::TraceLog(LOCATION, "Party ID: " + std::to_string(key.party_id), debug);
    utils::Logger::TraceLog(LOCATION, "(input size, element size) = (" + std::to_string(n) + ", " + std::to_string(e) + ")", debug);
    utils::Logger::TraceLog(LOCATION, "terminate size = " + std::to_string(nu), debug);
#endif

    // Get the seed and control bit from the DPF key.
    Block seed        = key.init_seed;
    bool  control_bit = key.party_id != 0;

    // line 3-8: definition
    std::array<Block, 2> expanded_seeds;           // expanded_seeds[keep or lose]
    std::array<bool, 2>  expanded_control_bits;    // expanded_control_bits[keep or lose]

    // Iterate over each level of the DPF protocol.
    for (uint32_t i = 0; i < nu; i++) {
        std::string current_level = "|Level=" + std::to_string(i) + "| ";

        EvaluateNextSeed(
            i, key.correction_words[i],
            seed, control_bit,
            expanded_seeds, expanded_control_bits);

        // line 6: Update the seed and control bit based on the current input bit.
        bool current_bit = (x & (1 << (n - i - 1))) != 0;

        if (current_bit) {    // current bit = 1
            seed        = expanded_seeds[kRight];
            control_bit = expanded_control_bits[kRight];
        } else {    // current bit = 0
            seed        = expanded_seeds[kLeft];
            control_bit = expanded_control_bits[kLeft];
        }

#ifdef LOG_LEVEL_TRACE
        utils::Logger::TraceLog(LOCATION, current_level + "Current bit: " + std::to_string(current_bit), debug);
        seed.PrintBlockHexTrace(LOCATION, current_level + "Next seed: ", debug);
        utils::Logger::TraceLog(LOCATION, current_level + "Next control bit: " + std::to_string(control_bit), debug);
#endif
    }

    // Calculate the final output based on the DPF protocol.
    Block    output_block = ComputeOutputBlock(seed, control_bit, key);
    uint32_t x_hat        = utils::GetLowerNBits(x, n - nu);
    uint32_t output       = output_block.ConvertVec(utils::Pow(2, n - nu), e)[x_hat];
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(output), debug);
#endif
    return output;
}

void DistributedPointFunction::EvaluateFullDomain(const DpfKey &key, std::vector<uint32_t> &outputs) const {
    uint32_t n  = this->params_.input_bitsize;
    uint32_t nu = this->params_.terminate_bitsize;

    if (n < 9) {
        FullDomainNonRecursive(key, outputs);
    } else if (n < 33 && n - nu == 2) {
        FullDomainNonRecursiveParallel_4(key, outputs);
    } else if (n < 17 && n - nu == 3) {
        FullDomainNonRecursiveParallel_8(key, outputs);
    } else {
        utils::Logger::FatalLog(LOCATION, "Unsupported input size: " + std::to_string(n) + ", terminate size: " + std::to_string(nu));
        exit(EXIT_FAILURE);
    }
}

void DistributedPointFunction::EvaluateFullDomainOneBit(const DpfKey &key, std::vector<uint32_t> &outputs) const {
    uint32_t n  = this->params_.input_bitsize;
    uint32_t e  = this->params_.element_bitsize;
    uint32_t nu = this->params_.terminate_bitsize;

    if (n < 8) {
        Block output_block = ComputeOutputBlock(key.init_seed, key.party_id != 0, key);
        outputs            = output_block.ConvertVec(utils::Pow(2, n - nu), e);
    } else if (n < 11) {
        FullDomainNonRecursive(key, outputs);
    } else if (n < 33 && n - nu == 7) {
        FullDomainNonRecursiveParallel_128(key, outputs);
    } else {
        utils::Logger::FatalLog(LOCATION, "Unsupported input size: " + std::to_string(n) + ", terminate size: " + std::to_string(nu));
        exit(EXIT_FAILURE);
    }
}

void DistributedPointFunction::FullDomainNonRecursive(const DpfKey &key, std::vector<uint32_t> &outputs) const {
    uint32_t n          = this->params_.input_bitsize;
    uint32_t e          = this->params_.element_bitsize;
    uint32_t nu         = this->params_.terminate_bitsize;
    uint32_t term_nodes = utils::Pow(2, n - nu);
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate FullDomainNonRecursive"), debug);
#endif

    // Get the seed and control bit from the DPF key.
    Block current_seed        = key.init_seed;
    bool  current_control_bit = key.party_id != 0;

    uint32_t idx   = 0;
    uint32_t depth = 0;
    uint32_t end   = utils::Pow(2, nu);

    Block              expanded_seed;
    bool               expanded_control_bit;
    Block              mask;
    std::vector<Block> prev_seed(nu + 1);
    std::vector<bool>  prev_control_bit(nu + 1);
    std::vector<Block> output_vec(end);

    prev_seed[0]        = current_seed;
    prev_control_bit[0] = current_control_bit;

    while (idx != end) {
        while (depth != nu) {
            bool keep           = (idx >> (nu - 1U - depth)) & 1U;
            current_seed        = prev_seed[depth];
            current_control_bit = prev_control_bit[depth];

            if (!keep) {    // Left
                prg_seed_left.Evaluate(current_seed, expanded_seed);
                expanded_control_bit = Lsb(expanded_seed);
                mask                 = zero_and_all_one[current_control_bit];
                current_seed         = expanded_seed ^ (mask & key.correction_words[depth].seed);
                current_control_bit  = expanded_control_bit ^ (current_control_bit & key.correction_words[depth].control_left);
            } else {    // Right
                prg_seed_right.Evaluate(current_seed, expanded_seed);
                expanded_control_bit = Lsb(expanded_seed);
                mask                 = zero_and_all_one[current_control_bit];
                current_seed         = expanded_seed ^ (mask & key.correction_words[depth].seed);
                current_control_bit  = expanded_control_bit ^ (current_control_bit & key.correction_words[depth].control_right);
            }
            depth++;
            prev_seed[depth]        = current_seed;
            prev_control_bit[depth] = current_control_bit;
        }
        output_vec[idx] = ComputeOutputBlock(current_seed, current_control_bit, key);

        int shift = (idx + 1U) ^ idx;
        depth -= static_cast<int>(std::floor(std::log2(shift))) + 1;
        idx++;
    }

    for (uint32_t i = 0; i < end; i++) {
        std::vector<uint32_t> output = output_vec[i].ConvertVec(term_nodes, e);
        for (uint32_t j = 0; j < term_nodes; j++) {
            outputs[i * term_nodes + j] = output[j];
        }
    }
}

void DistributedPointFunction::FullDomainNonRecursiveParallel_4(const DpfKey &key, std::vector<uint32_t> &outputs) const {
    uint32_t n          = this->params_.input_bitsize;
    uint32_t e          = this->params_.element_bitsize;
    uint32_t nu         = this->params_.terminate_bitsize;
    uint32_t term_nodes = utils::Pow(2, n - nu);

    if (term_nodes != 4) {
        utils::Logger::FatalLog(LOCATION, "The number of terminal nodes must be 4 (current: " + std::to_string(term_nodes) + ")");
        exit(EXIT_FAILURE);
    }
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate FullDomainNonRecursiveParallel_4"), debug);
#endif

    std::vector<Block> start_seeds{key.init_seed}, next_seeds;
    std::vector<bool>  start_control_bits{key.party_id != 0}, next_control_bits;
    for (int i = 0; i < 3; i++) {
        next_seeds.resize(utils::Pow(2, i + 1));
        next_control_bits.resize(utils::Pow(2, i + 1));
        for (size_t j = 0; j < start_seeds.size(); j++) {
            std::array<Block, 2> expanded_seeds;
            std::array<bool, 2>  expanded_control_bits;
            EvaluateNextSeed(i, key.correction_words[i], start_seeds[j], start_control_bits[j], expanded_seeds, expanded_control_bits);
            next_seeds[j * 2]            = expanded_seeds[kLeft];
            next_seeds[j * 2 + 1]        = expanded_seeds[kRight];
            next_control_bits[j * 2]     = expanded_control_bits[kLeft];
            next_control_bits[j * 2 + 1] = expanded_control_bits[kRight];
        }
        std::swap(start_seeds, next_seeds);
        std::swap(start_control_bits, next_control_bits);
    }

    uint32_t idx       = 0;
    uint32_t depth     = 0;
    uint32_t depth_end = nu - 3;
    uint32_t end       = utils::Pow(2, depth_end);
    uint32_t mask      = (1 << e) - 1;

    std::vector<std::array<Block, 8>> prev_seeds(depth_end + 1);
    std::vector<std::array<bool, 8>>  prev_control_bits(depth_end + 1);
    std::array<Block, 8>              expanded_seeds;
    std::array<bool, 8>               expanded_control_bits;
    std::array<Block, 8>              masks;
    std::array<Block, 8>              current_seeds;
    std::array<bool, 8>               current_control_bits = {false, false, false, false, false, false, false, false};

    prev_seeds[0][0] = start_seeds[0];
    prev_seeds[0][1] = start_seeds[1];
    prev_seeds[0][2] = start_seeds[2];
    prev_seeds[0][3] = start_seeds[3];
    prev_seeds[0][4] = start_seeds[4];
    prev_seeds[0][5] = start_seeds[5];
    prev_seeds[0][6] = start_seeds[6];
    prev_seeds[0][7] = start_seeds[7];

    prev_control_bits[0][0] = start_control_bits[0];
    prev_control_bits[0][1] = start_control_bits[1];
    prev_control_bits[0][2] = start_control_bits[2];
    prev_control_bits[0][3] = start_control_bits[3];
    prev_control_bits[0][4] = start_control_bits[4];
    prev_control_bits[0][5] = start_control_bits[5];
    prev_control_bits[0][6] = start_control_bits[6];
    prev_control_bits[0][7] = start_control_bits[7];

    while (idx != end) {
        while (depth != depth_end) {
            bool keep               = (idx >> (depth_end - 1U - depth)) & 1U;
            current_seeds[0]        = prev_seeds[depth][0];
            current_seeds[1]        = prev_seeds[depth][1];
            current_seeds[2]        = prev_seeds[depth][2];
            current_seeds[3]        = prev_seeds[depth][3];
            current_seeds[4]        = prev_seeds[depth][4];
            current_seeds[5]        = prev_seeds[depth][5];
            current_seeds[6]        = prev_seeds[depth][6];
            current_seeds[7]        = prev_seeds[depth][7];
            current_control_bits[0] = prev_control_bits[depth][0];
            current_control_bits[1] = prev_control_bits[depth][1];
            current_control_bits[2] = prev_control_bits[depth][2];
            current_control_bits[3] = prev_control_bits[depth][3];
            current_control_bits[4] = prev_control_bits[depth][4];
            current_control_bits[5] = prev_control_bits[depth][5];
            current_control_bits[6] = prev_control_bits[depth][6];
            current_control_bits[7] = prev_control_bits[depth][7];

            if (!keep) {    // Left
                prg_seed_left.Evaluate(current_seeds, expanded_seeds);

                expanded_control_bits[0] = Lsb(expanded_seeds[0]);
                expanded_control_bits[1] = Lsb(expanded_seeds[1]);
                expanded_control_bits[2] = Lsb(expanded_seeds[2]);
                expanded_control_bits[3] = Lsb(expanded_seeds[3]);
                expanded_control_bits[4] = Lsb(expanded_seeds[4]);
                expanded_control_bits[5] = Lsb(expanded_seeds[5]);
                expanded_control_bits[6] = Lsb(expanded_seeds[6]);
                expanded_control_bits[7] = Lsb(expanded_seeds[7]);

                masks[0] = zero_and_all_one[current_control_bits[0]];
                masks[1] = zero_and_all_one[current_control_bits[1]];
                masks[2] = zero_and_all_one[current_control_bits[2]];
                masks[3] = zero_and_all_one[current_control_bits[3]];
                masks[4] = zero_and_all_one[current_control_bits[4]];
                masks[5] = zero_and_all_one[current_control_bits[5]];
                masks[6] = zero_and_all_one[current_control_bits[6]];
                masks[7] = zero_and_all_one[current_control_bits[7]];

                current_seeds[0] = expanded_seeds[0] ^ (masks[0] & key.correction_words[depth + 3].seed);
                current_seeds[1] = expanded_seeds[1] ^ (masks[1] & key.correction_words[depth + 3].seed);
                current_seeds[2] = expanded_seeds[2] ^ (masks[2] & key.correction_words[depth + 3].seed);
                current_seeds[3] = expanded_seeds[3] ^ (masks[3] & key.correction_words[depth + 3].seed);
                current_seeds[4] = expanded_seeds[4] ^ (masks[4] & key.correction_words[depth + 3].seed);
                current_seeds[5] = expanded_seeds[5] ^ (masks[5] & key.correction_words[depth + 3].seed);
                current_seeds[6] = expanded_seeds[6] ^ (masks[6] & key.correction_words[depth + 3].seed);
                current_seeds[7] = expanded_seeds[7] ^ (masks[7] & key.correction_words[depth + 3].seed);

                current_control_bits[0] = expanded_control_bits[0] ^ (current_control_bits[0] & key.correction_words[depth + 3].control_left);
                current_control_bits[1] = expanded_control_bits[1] ^ (current_control_bits[1] & key.correction_words[depth + 3].control_left);
                current_control_bits[2] = expanded_control_bits[2] ^ (current_control_bits[2] & key.correction_words[depth + 3].control_left);
                current_control_bits[3] = expanded_control_bits[3] ^ (current_control_bits[3] & key.correction_words[depth + 3].control_left);
                current_control_bits[4] = expanded_control_bits[4] ^ (current_control_bits[4] & key.correction_words[depth + 3].control_left);
                current_control_bits[5] = expanded_control_bits[5] ^ (current_control_bits[5] & key.correction_words[depth + 3].control_left);
                current_control_bits[6] = expanded_control_bits[6] ^ (current_control_bits[6] & key.correction_words[depth + 3].control_left);
                current_control_bits[7] = expanded_control_bits[7] ^ (current_control_bits[7] & key.correction_words[depth + 3].control_left);
            } else {    // Right
                prg_seed_right.Evaluate(current_seeds, expanded_seeds);

                expanded_control_bits[0] = Lsb(expanded_seeds[0]);
                expanded_control_bits[1] = Lsb(expanded_seeds[1]);
                expanded_control_bits[2] = Lsb(expanded_seeds[2]);
                expanded_control_bits[3] = Lsb(expanded_seeds[3]);
                expanded_control_bits[4] = Lsb(expanded_seeds[4]);
                expanded_control_bits[5] = Lsb(expanded_seeds[5]);
                expanded_control_bits[6] = Lsb(expanded_seeds[6]);
                expanded_control_bits[7] = Lsb(expanded_seeds[7]);

                masks[0] = zero_and_all_one[current_control_bits[0]];
                masks[1] = zero_and_all_one[current_control_bits[1]];
                masks[2] = zero_and_all_one[current_control_bits[2]];
                masks[3] = zero_and_all_one[current_control_bits[3]];
                masks[4] = zero_and_all_one[current_control_bits[4]];
                masks[5] = zero_and_all_one[current_control_bits[5]];
                masks[6] = zero_and_all_one[current_control_bits[6]];
                masks[7] = zero_and_all_one[current_control_bits[7]];

                current_seeds[0] = expanded_seeds[0] ^ (masks[0] & key.correction_words[depth + 3].seed);
                current_seeds[1] = expanded_seeds[1] ^ (masks[1] & key.correction_words[depth + 3].seed);
                current_seeds[2] = expanded_seeds[2] ^ (masks[2] & key.correction_words[depth + 3].seed);
                current_seeds[3] = expanded_seeds[3] ^ (masks[3] & key.correction_words[depth + 3].seed);
                current_seeds[4] = expanded_seeds[4] ^ (masks[4] & key.correction_words[depth + 3].seed);
                current_seeds[5] = expanded_seeds[5] ^ (masks[5] & key.correction_words[depth + 3].seed);
                current_seeds[6] = expanded_seeds[6] ^ (masks[6] & key.correction_words[depth + 3].seed);
                current_seeds[7] = expanded_seeds[7] ^ (masks[7] & key.correction_words[depth + 3].seed);

                current_control_bits[0] = expanded_control_bits[0] ^ (current_control_bits[0] & key.correction_words[depth + 3].control_right);
                current_control_bits[1] = expanded_control_bits[1] ^ (current_control_bits[1] & key.correction_words[depth + 3].control_right);
                current_control_bits[2] = expanded_control_bits[2] ^ (current_control_bits[2] & key.correction_words[depth + 3].control_right);
                current_control_bits[3] = expanded_control_bits[3] ^ (current_control_bits[3] & key.correction_words[depth + 3].control_right);
                current_control_bits[4] = expanded_control_bits[4] ^ (current_control_bits[4] & key.correction_words[depth + 3].control_right);
                current_control_bits[5] = expanded_control_bits[5] ^ (current_control_bits[5] & key.correction_words[depth + 3].control_right);
                current_control_bits[6] = expanded_control_bits[6] ^ (current_control_bits[6] & key.correction_words[depth + 3].control_right);
                current_control_bits[7] = expanded_control_bits[7] ^ (current_control_bits[7] & key.correction_words[depth + 3].control_right);
            }
            depth++;
            prev_seeds[depth][0] = current_seeds[0];
            prev_seeds[depth][1] = current_seeds[1];
            prev_seeds[depth][2] = current_seeds[2];
            prev_seeds[depth][3] = current_seeds[3];
            prev_seeds[depth][4] = current_seeds[4];
            prev_seeds[depth][5] = current_seeds[5];
            prev_seeds[depth][6] = current_seeds[6];
            prev_seeds[depth][7] = current_seeds[7];

            prev_control_bits[depth][0] = current_control_bits[0];
            prev_control_bits[depth][1] = current_control_bits[1];
            prev_control_bits[depth][2] = current_control_bits[2];
            prev_control_bits[depth][3] = current_control_bits[3];
            prev_control_bits[depth][4] = current_control_bits[4];
            prev_control_bits[depth][5] = current_control_bits[5];
            prev_control_bits[depth][6] = current_control_bits[6];
            prev_control_bits[depth][7] = current_control_bits[7];
        }

        masks[0] = zero_and_all_one[current_control_bits[0]];
        masks[1] = zero_and_all_one[current_control_bits[1]];
        masks[2] = zero_and_all_one[current_control_bits[2]];
        masks[3] = zero_and_all_one[current_control_bits[3]];
        masks[4] = zero_and_all_one[current_control_bits[4]];
        masks[5] = zero_and_all_one[current_control_bits[5]];
        masks[6] = zero_and_all_one[current_control_bits[6]];
        masks[7] = zero_and_all_one[current_control_bits[7]];

        std::array<Block, 8> output_blocks = {zero_block, zero_block, zero_block, zero_block, zero_block, zero_block, zero_block, zero_block};
        if (key.party_id) {
            output_blocks[0] = _mm_sub_epi32(zero_block, _mm_add_epi32(current_seeds[0], masks[0] & key.output));
            output_blocks[1] = _mm_sub_epi32(zero_block, _mm_add_epi32(current_seeds[1], masks[1] & key.output));
            output_blocks[2] = _mm_sub_epi32(zero_block, _mm_add_epi32(current_seeds[2], masks[2] & key.output));
            output_blocks[3] = _mm_sub_epi32(zero_block, _mm_add_epi32(current_seeds[3], masks[3] & key.output));
            output_blocks[4] = _mm_sub_epi32(zero_block, _mm_add_epi32(current_seeds[4], masks[4] & key.output));
            output_blocks[5] = _mm_sub_epi32(zero_block, _mm_add_epi32(current_seeds[5], masks[5] & key.output));
            output_blocks[6] = _mm_sub_epi32(zero_block, _mm_add_epi32(current_seeds[6], masks[6] & key.output));
            output_blocks[7] = _mm_sub_epi32(zero_block, _mm_add_epi32(current_seeds[7], masks[7] & key.output));
        } else {
            output_blocks[0] = _mm_add_epi32(current_seeds[0], masks[0] & key.output);
            output_blocks[1] = _mm_add_epi32(current_seeds[1], masks[1] & key.output);
            output_blocks[2] = _mm_add_epi32(current_seeds[2], masks[2] & key.output);
            output_blocks[3] = _mm_add_epi32(current_seeds[3], masks[3] & key.output);
            output_blocks[4] = _mm_add_epi32(current_seeds[4], masks[4] & key.output);
            output_blocks[5] = _mm_add_epi32(current_seeds[5], masks[5] & key.output);
            output_blocks[6] = _mm_add_epi32(current_seeds[6], masks[6] & key.output);
            output_blocks[7] = _mm_add_epi32(current_seeds[7], masks[7] & key.output);
        }

        uint32_t start     = 0 * utils::Pow(2, n - 3) + idx * 4;
        outputs[start + 0] = _mm_extract_epi32(output_blocks[0], 0) & mask;
        outputs[start + 1] = _mm_extract_epi32(output_blocks[0], 1) & mask;
        outputs[start + 2] = _mm_extract_epi32(output_blocks[0], 2) & mask;
        outputs[start + 3] = _mm_extract_epi32(output_blocks[0], 3) & mask;
        start              = 1 * utils::Pow(2, n - 3) + idx * 4;
        outputs[start + 0] = _mm_extract_epi32(output_blocks[1], 0) & mask;
        outputs[start + 1] = _mm_extract_epi32(output_blocks[1], 1) & mask;
        outputs[start + 2] = _mm_extract_epi32(output_blocks[1], 2) & mask;
        outputs[start + 3] = _mm_extract_epi32(output_blocks[1], 3) & mask;
        start              = 2 * utils::Pow(2, n - 3) + idx * 4;
        outputs[start + 0] = _mm_extract_epi32(output_blocks[2], 0) & mask;
        outputs[start + 1] = _mm_extract_epi32(output_blocks[2], 1) & mask;
        outputs[start + 2] = _mm_extract_epi32(output_blocks[2], 2) & mask;
        outputs[start + 3] = _mm_extract_epi32(output_blocks[2], 3) & mask;
        start              = 3 * utils::Pow(2, n - 3) + idx * 4;
        outputs[start + 0] = _mm_extract_epi32(output_blocks[3], 0) & mask;
        outputs[start + 1] = _mm_extract_epi32(output_blocks[3], 1) & mask;
        outputs[start + 2] = _mm_extract_epi32(output_blocks[3], 2) & mask;
        outputs[start + 3] = _mm_extract_epi32(output_blocks[3], 3) & mask;
        start              = 4 * utils::Pow(2, n - 3) + idx * 4;
        outputs[start + 0] = _mm_extract_epi32(output_blocks[4], 0) & mask;
        outputs[start + 1] = _mm_extract_epi32(output_blocks[4], 1) & mask;
        outputs[start + 2] = _mm_extract_epi32(output_blocks[4], 2) & mask;
        outputs[start + 3] = _mm_extract_epi32(output_blocks[4], 3) & mask;
        start              = 5 * utils::Pow(2, n - 3) + idx * 4;
        outputs[start + 0] = _mm_extract_epi32(output_blocks[5], 0) & mask;
        outputs[start + 1] = _mm_extract_epi32(output_blocks[5], 1) & mask;
        outputs[start + 2] = _mm_extract_epi32(output_blocks[5], 2) & mask;
        outputs[start + 3] = _mm_extract_epi32(output_blocks[5], 3) & mask;
        start              = 6 * utils::Pow(2, n - 3) + idx * 4;
        outputs[start + 0] = _mm_extract_epi32(output_blocks[6], 0) & mask;
        outputs[start + 1] = _mm_extract_epi32(output_blocks[6], 1) & mask;
        outputs[start + 2] = _mm_extract_epi32(output_blocks[6], 2) & mask;
        outputs[start + 3] = _mm_extract_epi32(output_blocks[6], 3) & mask;
        start              = 7 * utils::Pow(2, n - 3) + idx * 4;
        outputs[start + 0] = _mm_extract_epi32(output_blocks[7], 0) & mask;
        outputs[start + 1] = _mm_extract_epi32(output_blocks[7], 1) & mask;
        outputs[start + 2] = _mm_extract_epi32(output_blocks[7], 2) & mask;
        outputs[start + 3] = _mm_extract_epi32(output_blocks[7], 3) & mask;

        int shift = (idx + 1U) ^ idx;
        depth -= static_cast<int>(std::floor(std::log2(shift))) + 1;
        idx++;
    }
}

void DistributedPointFunction::FullDomainNonRecursiveParallel_8(const DpfKey &key, std::vector<uint32_t> &outputs) const {
    uint32_t n          = this->params_.input_bitsize;
    uint32_t e          = this->params_.element_bitsize;
    uint32_t nu         = this->params_.terminate_bitsize;
    uint32_t term_nodes = utils::Pow(2, n - nu);

    if (term_nodes != 8) {
        utils::Logger::FatalLog(LOCATION, "The number of terminal nodes must be 8 (current: " + std::to_string(term_nodes) + ")");
        exit(EXIT_FAILURE);
    }
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate FullDomainNonRecursiveParallel_8"), debug);
#endif

    std::vector<Block> start_seeds{key.init_seed}, next_seeds;
    std::vector<bool>  start_control_bits{key.party_id != 0}, next_control_bits;
    for (int i = 0; i < 3; i++) {
        next_seeds.resize(utils::Pow(2, i + 1));
        next_control_bits.resize(utils::Pow(2, i + 1));
        for (size_t j = 0; j < start_seeds.size(); j++) {
            std::array<Block, 2> expanded_seeds;
            std::array<bool, 2>  expanded_control_bits;
            EvaluateNextSeed(i, key.correction_words[i], start_seeds[j], start_control_bits[j], expanded_seeds, expanded_control_bits);
            next_seeds[j * 2]            = expanded_seeds[kLeft];
            next_seeds[j * 2 + 1]        = expanded_seeds[kRight];
            next_control_bits[j * 2]     = expanded_control_bits[kLeft];
            next_control_bits[j * 2 + 1] = expanded_control_bits[kRight];
        }
        std::swap(start_seeds, next_seeds);
        std::swap(start_control_bits, next_control_bits);
    }

    uint32_t idx       = 0;
    uint32_t depth     = 0;
    uint32_t depth_end = nu - 3;
    uint32_t end       = utils::Pow(2, depth_end);
    uint32_t mask      = (1 << e) - 1;

    std::vector<std::array<Block, 8>> prev_seeds(depth_end + 1);
    std::vector<std::array<bool, 8>>  prev_control_bits(depth_end + 1);
    std::array<Block, 8>              expanded_seeds;
    std::array<bool, 8>               expanded_control_bits;
    std::array<Block, 8>              masks;
    std::array<Block, 8>              current_seeds;
    std::array<bool, 8>               current_control_bits = {false, false, false, false, false, false, false, false};

    prev_seeds[0][0] = start_seeds[0];
    prev_seeds[0][1] = start_seeds[1];
    prev_seeds[0][2] = start_seeds[2];
    prev_seeds[0][3] = start_seeds[3];
    prev_seeds[0][4] = start_seeds[4];
    prev_seeds[0][5] = start_seeds[5];
    prev_seeds[0][6] = start_seeds[6];
    prev_seeds[0][7] = start_seeds[7];

    prev_control_bits[0][0] = start_control_bits[0];
    prev_control_bits[0][1] = start_control_bits[1];
    prev_control_bits[0][2] = start_control_bits[2];
    prev_control_bits[0][3] = start_control_bits[3];
    prev_control_bits[0][4] = start_control_bits[4];
    prev_control_bits[0][5] = start_control_bits[5];
    prev_control_bits[0][6] = start_control_bits[6];
    prev_control_bits[0][7] = start_control_bits[7];

    while (idx != end) {
        while (depth != depth_end) {
            bool keep               = (idx >> (depth_end - 1U - depth)) & 1U;
            current_seeds[0]        = prev_seeds[depth][0];
            current_seeds[1]        = prev_seeds[depth][1];
            current_seeds[2]        = prev_seeds[depth][2];
            current_seeds[3]        = prev_seeds[depth][3];
            current_seeds[4]        = prev_seeds[depth][4];
            current_seeds[5]        = prev_seeds[depth][5];
            current_seeds[6]        = prev_seeds[depth][6];
            current_seeds[7]        = prev_seeds[depth][7];
            current_control_bits[0] = prev_control_bits[depth][0];
            current_control_bits[1] = prev_control_bits[depth][1];
            current_control_bits[2] = prev_control_bits[depth][2];
            current_control_bits[3] = prev_control_bits[depth][3];
            current_control_bits[4] = prev_control_bits[depth][4];
            current_control_bits[5] = prev_control_bits[depth][5];
            current_control_bits[6] = prev_control_bits[depth][6];
            current_control_bits[7] = prev_control_bits[depth][7];

            if (!keep) {    // Left
                prg_seed_left.Evaluate(current_seeds, expanded_seeds);

                expanded_control_bits[0] = Lsb(expanded_seeds[0]);
                expanded_control_bits[1] = Lsb(expanded_seeds[1]);
                expanded_control_bits[2] = Lsb(expanded_seeds[2]);
                expanded_control_bits[3] = Lsb(expanded_seeds[3]);
                expanded_control_bits[4] = Lsb(expanded_seeds[4]);
                expanded_control_bits[5] = Lsb(expanded_seeds[5]);
                expanded_control_bits[6] = Lsb(expanded_seeds[6]);
                expanded_control_bits[7] = Lsb(expanded_seeds[7]);

                masks[0] = zero_and_all_one[current_control_bits[0]];
                masks[1] = zero_and_all_one[current_control_bits[1]];
                masks[2] = zero_and_all_one[current_control_bits[2]];
                masks[3] = zero_and_all_one[current_control_bits[3]];
                masks[4] = zero_and_all_one[current_control_bits[4]];
                masks[5] = zero_and_all_one[current_control_bits[5]];
                masks[6] = zero_and_all_one[current_control_bits[6]];
                masks[7] = zero_and_all_one[current_control_bits[7]];

                current_seeds[0] = expanded_seeds[0] ^ (masks[0] & key.correction_words[depth + 3].seed);
                current_seeds[1] = expanded_seeds[1] ^ (masks[1] & key.correction_words[depth + 3].seed);
                current_seeds[2] = expanded_seeds[2] ^ (masks[2] & key.correction_words[depth + 3].seed);
                current_seeds[3] = expanded_seeds[3] ^ (masks[3] & key.correction_words[depth + 3].seed);
                current_seeds[4] = expanded_seeds[4] ^ (masks[4] & key.correction_words[depth + 3].seed);
                current_seeds[5] = expanded_seeds[5] ^ (masks[5] & key.correction_words[depth + 3].seed);
                current_seeds[6] = expanded_seeds[6] ^ (masks[6] & key.correction_words[depth + 3].seed);
                current_seeds[7] = expanded_seeds[7] ^ (masks[7] & key.correction_words[depth + 3].seed);

                current_control_bits[0] = expanded_control_bits[0] ^ (current_control_bits[0] & key.correction_words[depth + 3].control_left);
                current_control_bits[1] = expanded_control_bits[1] ^ (current_control_bits[1] & key.correction_words[depth + 3].control_left);
                current_control_bits[2] = expanded_control_bits[2] ^ (current_control_bits[2] & key.correction_words[depth + 3].control_left);
                current_control_bits[3] = expanded_control_bits[3] ^ (current_control_bits[3] & key.correction_words[depth + 3].control_left);
                current_control_bits[4] = expanded_control_bits[4] ^ (current_control_bits[4] & key.correction_words[depth + 3].control_left);
                current_control_bits[5] = expanded_control_bits[5] ^ (current_control_bits[5] & key.correction_words[depth + 3].control_left);
                current_control_bits[6] = expanded_control_bits[6] ^ (current_control_bits[6] & key.correction_words[depth + 3].control_left);
                current_control_bits[7] = expanded_control_bits[7] ^ (current_control_bits[7] & key.correction_words[depth + 3].control_left);
            } else {    // Right
                prg_seed_right.Evaluate(current_seeds, expanded_seeds);

                expanded_control_bits[0] = Lsb(expanded_seeds[0]);
                expanded_control_bits[1] = Lsb(expanded_seeds[1]);
                expanded_control_bits[2] = Lsb(expanded_seeds[2]);
                expanded_control_bits[3] = Lsb(expanded_seeds[3]);
                expanded_control_bits[4] = Lsb(expanded_seeds[4]);
                expanded_control_bits[5] = Lsb(expanded_seeds[5]);
                expanded_control_bits[6] = Lsb(expanded_seeds[6]);
                expanded_control_bits[7] = Lsb(expanded_seeds[7]);

                masks[0] = zero_and_all_one[current_control_bits[0]];
                masks[1] = zero_and_all_one[current_control_bits[1]];
                masks[2] = zero_and_all_one[current_control_bits[2]];
                masks[3] = zero_and_all_one[current_control_bits[3]];
                masks[4] = zero_and_all_one[current_control_bits[4]];
                masks[5] = zero_and_all_one[current_control_bits[5]];
                masks[6] = zero_and_all_one[current_control_bits[6]];
                masks[7] = zero_and_all_one[current_control_bits[7]];

                current_seeds[0] = expanded_seeds[0] ^ (masks[0] & key.correction_words[depth + 3].seed);
                current_seeds[1] = expanded_seeds[1] ^ (masks[1] & key.correction_words[depth + 3].seed);
                current_seeds[2] = expanded_seeds[2] ^ (masks[2] & key.correction_words[depth + 3].seed);
                current_seeds[3] = expanded_seeds[3] ^ (masks[3] & key.correction_words[depth + 3].seed);
                current_seeds[4] = expanded_seeds[4] ^ (masks[4] & key.correction_words[depth + 3].seed);
                current_seeds[5] = expanded_seeds[5] ^ (masks[5] & key.correction_words[depth + 3].seed);
                current_seeds[6] = expanded_seeds[6] ^ (masks[6] & key.correction_words[depth + 3].seed);
                current_seeds[7] = expanded_seeds[7] ^ (masks[7] & key.correction_words[depth + 3].seed);

                current_control_bits[0] = expanded_control_bits[0] ^ (current_control_bits[0] & key.correction_words[depth + 3].control_right);
                current_control_bits[1] = expanded_control_bits[1] ^ (current_control_bits[1] & key.correction_words[depth + 3].control_right);
                current_control_bits[2] = expanded_control_bits[2] ^ (current_control_bits[2] & key.correction_words[depth + 3].control_right);
                current_control_bits[3] = expanded_control_bits[3] ^ (current_control_bits[3] & key.correction_words[depth + 3].control_right);
                current_control_bits[4] = expanded_control_bits[4] ^ (current_control_bits[4] & key.correction_words[depth + 3].control_right);
                current_control_bits[5] = expanded_control_bits[5] ^ (current_control_bits[5] & key.correction_words[depth + 3].control_right);
                current_control_bits[6] = expanded_control_bits[6] ^ (current_control_bits[6] & key.correction_words[depth + 3].control_right);
                current_control_bits[7] = expanded_control_bits[7] ^ (current_control_bits[7] & key.correction_words[depth + 3].control_right);
            }
            depth++;
            prev_seeds[depth][0] = current_seeds[0];
            prev_seeds[depth][1] = current_seeds[1];
            prev_seeds[depth][2] = current_seeds[2];
            prev_seeds[depth][3] = current_seeds[3];
            prev_seeds[depth][4] = current_seeds[4];
            prev_seeds[depth][5] = current_seeds[5];
            prev_seeds[depth][6] = current_seeds[6];
            prev_seeds[depth][7] = current_seeds[7];

            prev_control_bits[depth][0] = current_control_bits[0];
            prev_control_bits[depth][1] = current_control_bits[1];
            prev_control_bits[depth][2] = current_control_bits[2];
            prev_control_bits[depth][3] = current_control_bits[3];
            prev_control_bits[depth][4] = current_control_bits[4];
            prev_control_bits[depth][5] = current_control_bits[5];
            prev_control_bits[depth][6] = current_control_bits[6];
            prev_control_bits[depth][7] = current_control_bits[7];
        }

        masks[0] = zero_and_all_one[current_control_bits[0]];
        masks[1] = zero_and_all_one[current_control_bits[1]];
        masks[2] = zero_and_all_one[current_control_bits[2]];
        masks[3] = zero_and_all_one[current_control_bits[3]];
        masks[4] = zero_and_all_one[current_control_bits[4]];
        masks[5] = zero_and_all_one[current_control_bits[5]];
        masks[6] = zero_and_all_one[current_control_bits[6]];
        masks[7] = zero_and_all_one[current_control_bits[7]];

        std::array<Block, 8> output_blocks = {zero_block, zero_block, zero_block, zero_block, zero_block, zero_block, zero_block, zero_block};
        if (key.party_id) {
            output_blocks[0] = _mm_sub_epi16(zero_block, _mm_add_epi16(current_seeds[0], masks[0] & key.output));
            output_blocks[1] = _mm_sub_epi16(zero_block, _mm_add_epi16(current_seeds[1], masks[1] & key.output));
            output_blocks[2] = _mm_sub_epi16(zero_block, _mm_add_epi16(current_seeds[2], masks[2] & key.output));
            output_blocks[3] = _mm_sub_epi16(zero_block, _mm_add_epi16(current_seeds[3], masks[3] & key.output));
            output_blocks[4] = _mm_sub_epi16(zero_block, _mm_add_epi16(current_seeds[4], masks[4] & key.output));
            output_blocks[5] = _mm_sub_epi16(zero_block, _mm_add_epi16(current_seeds[5], masks[5] & key.output));
            output_blocks[6] = _mm_sub_epi16(zero_block, _mm_add_epi16(current_seeds[6], masks[6] & key.output));
            output_blocks[7] = _mm_sub_epi16(zero_block, _mm_add_epi16(current_seeds[7], masks[7] & key.output));
        } else {
            output_blocks[0] = _mm_add_epi16(current_seeds[0], masks[0] & key.output);
            output_blocks[1] = _mm_add_epi16(current_seeds[1], masks[1] & key.output);
            output_blocks[2] = _mm_add_epi16(current_seeds[2], masks[2] & key.output);
            output_blocks[3] = _mm_add_epi16(current_seeds[3], masks[3] & key.output);
            output_blocks[4] = _mm_add_epi16(current_seeds[4], masks[4] & key.output);
            output_blocks[5] = _mm_add_epi16(current_seeds[5], masks[5] & key.output);
            output_blocks[6] = _mm_add_epi16(current_seeds[6], masks[6] & key.output);
            output_blocks[7] = _mm_add_epi16(current_seeds[7], masks[7] & key.output);
        }

        uint32_t start     = 0 * utils::Pow(2, n - 3) + idx * 8;
        outputs[start + 0] = _mm_extract_epi16(output_blocks[0], 0) & mask;
        outputs[start + 1] = _mm_extract_epi16(output_blocks[0], 1) & mask;
        outputs[start + 2] = _mm_extract_epi16(output_blocks[0], 2) & mask;
        outputs[start + 3] = _mm_extract_epi16(output_blocks[0], 3) & mask;
        outputs[start + 4] = _mm_extract_epi16(output_blocks[0], 4) & mask;
        outputs[start + 5] = _mm_extract_epi16(output_blocks[0], 5) & mask;
        outputs[start + 6] = _mm_extract_epi16(output_blocks[0], 6) & mask;
        outputs[start + 7] = _mm_extract_epi16(output_blocks[0], 7) & mask;
        start              = 1 * utils::Pow(2, n - 3) + idx * 8;
        outputs[start + 0] = _mm_extract_epi16(output_blocks[1], 0) & mask;
        outputs[start + 1] = _mm_extract_epi16(output_blocks[1], 1) & mask;
        outputs[start + 2] = _mm_extract_epi16(output_blocks[1], 2) & mask;
        outputs[start + 3] = _mm_extract_epi16(output_blocks[1], 3) & mask;
        outputs[start + 4] = _mm_extract_epi16(output_blocks[1], 4) & mask;
        outputs[start + 5] = _mm_extract_epi16(output_blocks[1], 5) & mask;
        outputs[start + 6] = _mm_extract_epi16(output_blocks[1], 6) & mask;
        outputs[start + 7] = _mm_extract_epi16(output_blocks[1], 7) & mask;
        start              = 2 * utils::Pow(2, n - 3) + idx * 8;
        outputs[start + 0] = _mm_extract_epi16(output_blocks[2], 0) & mask;
        outputs[start + 1] = _mm_extract_epi16(output_blocks[2], 1) & mask;
        outputs[start + 2] = _mm_extract_epi16(output_blocks[2], 2) & mask;
        outputs[start + 3] = _mm_extract_epi16(output_blocks[2], 3) & mask;
        outputs[start + 4] = _mm_extract_epi16(output_blocks[2], 4) & mask;
        outputs[start + 5] = _mm_extract_epi16(output_blocks[2], 5) & mask;
        outputs[start + 6] = _mm_extract_epi16(output_blocks[2], 6) & mask;
        outputs[start + 7] = _mm_extract_epi16(output_blocks[2], 7) & mask;
        start              = 3 * utils::Pow(2, n - 3) + idx * 8;
        outputs[start + 0] = _mm_extract_epi16(output_blocks[3], 0) & mask;
        outputs[start + 1] = _mm_extract_epi16(output_blocks[3], 1) & mask;
        outputs[start + 2] = _mm_extract_epi16(output_blocks[3], 2) & mask;
        outputs[start + 3] = _mm_extract_epi16(output_blocks[3], 3) & mask;
        outputs[start + 4] = _mm_extract_epi16(output_blocks[3], 4) & mask;
        outputs[start + 5] = _mm_extract_epi16(output_blocks[3], 5) & mask;
        outputs[start + 6] = _mm_extract_epi16(output_blocks[3], 6) & mask;
        outputs[start + 7] = _mm_extract_epi16(output_blocks[3], 7) & mask;
        start              = 4 * utils::Pow(2, n - 3) + idx * 8;
        outputs[start + 0] = _mm_extract_epi16(output_blocks[4], 0) & mask;
        outputs[start + 1] = _mm_extract_epi16(output_blocks[4], 1) & mask;
        outputs[start + 2] = _mm_extract_epi16(output_blocks[4], 2) & mask;
        outputs[start + 3] = _mm_extract_epi16(output_blocks[4], 3) & mask;
        outputs[start + 4] = _mm_extract_epi16(output_blocks[4], 4) & mask;
        outputs[start + 5] = _mm_extract_epi16(output_blocks[4], 5) & mask;
        outputs[start + 6] = _mm_extract_epi16(output_blocks[4], 6) & mask;
        outputs[start + 7] = _mm_extract_epi16(output_blocks[4], 7) & mask;
        start              = 5 * utils::Pow(2, n - 3) + idx * 8;
        outputs[start + 0] = _mm_extract_epi16(output_blocks[5], 0) & mask;
        outputs[start + 1] = _mm_extract_epi16(output_blocks[5], 1) & mask;
        outputs[start + 2] = _mm_extract_epi16(output_blocks[5], 2) & mask;
        outputs[start + 3] = _mm_extract_epi16(output_blocks[5], 3) & mask;
        outputs[start + 4] = _mm_extract_epi16(output_blocks[5], 4) & mask;
        outputs[start + 5] = _mm_extract_epi16(output_blocks[5], 5) & mask;
        outputs[start + 6] = _mm_extract_epi16(output_blocks[5], 6) & mask;
        outputs[start + 7] = _mm_extract_epi16(output_blocks[5], 7) & mask;
        start              = 6 * utils::Pow(2, n - 3) + idx * 8;
        outputs[start + 0] = _mm_extract_epi16(output_blocks[6], 0) & mask;
        outputs[start + 1] = _mm_extract_epi16(output_blocks[6], 1) & mask;
        outputs[start + 2] = _mm_extract_epi16(output_blocks[6], 2) & mask;
        outputs[start + 3] = _mm_extract_epi16(output_blocks[6], 3) & mask;
        outputs[start + 4] = _mm_extract_epi16(output_blocks[6], 4) & mask;
        outputs[start + 5] = _mm_extract_epi16(output_blocks[6], 5) & mask;
        outputs[start + 6] = _mm_extract_epi16(output_blocks[6], 6) & mask;
        outputs[start + 7] = _mm_extract_epi16(output_blocks[6], 7) & mask;
        start              = 7 * utils::Pow(2, n - 3) + idx * 8;
        outputs[start + 0] = _mm_extract_epi16(output_blocks[7], 0) & mask;
        outputs[start + 1] = _mm_extract_epi16(output_blocks[7], 1) & mask;
        outputs[start + 2] = _mm_extract_epi16(output_blocks[7], 2) & mask;
        outputs[start + 3] = _mm_extract_epi16(output_blocks[7], 3) & mask;
        outputs[start + 4] = _mm_extract_epi16(output_blocks[7], 4) & mask;
        outputs[start + 5] = _mm_extract_epi16(output_blocks[7], 5) & mask;
        outputs[start + 6] = _mm_extract_epi16(output_blocks[7], 6) & mask;
        outputs[start + 7] = _mm_extract_epi16(output_blocks[7], 7) & mask;

        int shift = (idx + 1U) ^ idx;
        depth -= static_cast<int>(std::floor(std::log2(shift))) + 1;
        idx++;
    }
}

void DistributedPointFunction::FullDomainNonRecursiveParallel_128(const DpfKey &key, std::vector<uint32_t> &outputs) const {
    uint32_t n          = this->params_.input_bitsize;
    uint32_t e          = this->params_.element_bitsize;
    uint32_t nu         = this->params_.terminate_bitsize;
    uint32_t term_nodes = utils::Pow(2, n - nu);

    if (term_nodes != 128) {
        utils::Logger::FatalLog(LOCATION, "The number of terminal nodes must be 128 (current: " + std::to_string(term_nodes) + ")");
        exit(EXIT_FAILURE);
    }
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate FullDomainNonRecursiveParallel_128"), debug);
#endif

    std::vector<Block> start_seeds{key.init_seed}, next_seeds;
    std::vector<bool>  start_control_bits{key.party_id != 0}, next_control_bits;
    for (int i = 0; i < 3; i++) {
        next_seeds.resize(utils::Pow(2, i + 1));
        next_control_bits.resize(utils::Pow(2, i + 1));
        for (size_t j = 0; j < start_seeds.size(); j++) {
            std::array<Block, 2> expanded_seeds;
            std::array<bool, 2>  expanded_control_bits;
            EvaluateNextSeed(i, key.correction_words[i], start_seeds[j], start_control_bits[j], expanded_seeds, expanded_control_bits);
            next_seeds[j * 2]            = expanded_seeds[kLeft];
            next_seeds[j * 2 + 1]        = expanded_seeds[kRight];
            next_control_bits[j * 2]     = expanded_control_bits[kLeft];
            next_control_bits[j * 2 + 1] = expanded_control_bits[kRight];
        }
        std::swap(start_seeds, next_seeds);
        std::swap(start_control_bits, next_control_bits);
    }

    uint32_t idx       = 0;
    uint32_t depth     = 0;
    uint32_t depth_end = nu - 3;
    uint32_t end       = utils::Pow(2, depth_end);

    std::vector<std::array<Block, 8>> prev_seeds(depth_end + 1);
    std::vector<std::array<bool, 8>>  prev_control_bits(depth_end + 1);
    std::array<Block, 8>              expanded_seeds;
    std::array<bool, 8>               expanded_control_bits;
    std::array<Block, 8>              masks;
    std::array<Block, 8>              current_seeds;
    std::array<bool, 8>               current_control_bits = {false, false, false, false, false, false, false, false};
    std::vector<std::array<Block, 8>> output_vec(end);

    prev_seeds[0][0] = start_seeds[0];
    prev_seeds[0][1] = start_seeds[1];
    prev_seeds[0][2] = start_seeds[2];
    prev_seeds[0][3] = start_seeds[3];
    prev_seeds[0][4] = start_seeds[4];
    prev_seeds[0][5] = start_seeds[5];
    prev_seeds[0][6] = start_seeds[6];
    prev_seeds[0][7] = start_seeds[7];

    prev_control_bits[0][0] = start_control_bits[0];
    prev_control_bits[0][1] = start_control_bits[1];
    prev_control_bits[0][2] = start_control_bits[2];
    prev_control_bits[0][3] = start_control_bits[3];
    prev_control_bits[0][4] = start_control_bits[4];
    prev_control_bits[0][5] = start_control_bits[5];
    prev_control_bits[0][6] = start_control_bits[6];
    prev_control_bits[0][7] = start_control_bits[7];

    while (idx != end) {
        while (depth != depth_end) {
            bool keep               = (idx >> (depth_end - 1U - depth)) & 1U;
            current_seeds[0]        = prev_seeds[depth][0];
            current_seeds[1]        = prev_seeds[depth][1];
            current_seeds[2]        = prev_seeds[depth][2];
            current_seeds[3]        = prev_seeds[depth][3];
            current_seeds[4]        = prev_seeds[depth][4];
            current_seeds[5]        = prev_seeds[depth][5];
            current_seeds[6]        = prev_seeds[depth][6];
            current_seeds[7]        = prev_seeds[depth][7];
            current_control_bits[0] = prev_control_bits[depth][0];
            current_control_bits[1] = prev_control_bits[depth][1];
            current_control_bits[2] = prev_control_bits[depth][2];
            current_control_bits[3] = prev_control_bits[depth][3];
            current_control_bits[4] = prev_control_bits[depth][4];
            current_control_bits[5] = prev_control_bits[depth][5];
            current_control_bits[6] = prev_control_bits[depth][6];
            current_control_bits[7] = prev_control_bits[depth][7];

            if (!keep) {    // Left
                prg_seed_left.Evaluate(current_seeds, expanded_seeds);

                expanded_control_bits[0] = Lsb(expanded_seeds[0]);
                expanded_control_bits[1] = Lsb(expanded_seeds[1]);
                expanded_control_bits[2] = Lsb(expanded_seeds[2]);
                expanded_control_bits[3] = Lsb(expanded_seeds[3]);
                expanded_control_bits[4] = Lsb(expanded_seeds[4]);
                expanded_control_bits[5] = Lsb(expanded_seeds[5]);
                expanded_control_bits[6] = Lsb(expanded_seeds[6]);
                expanded_control_bits[7] = Lsb(expanded_seeds[7]);

                masks[0] = zero_and_all_one[current_control_bits[0]];
                masks[1] = zero_and_all_one[current_control_bits[1]];
                masks[2] = zero_and_all_one[current_control_bits[2]];
                masks[3] = zero_and_all_one[current_control_bits[3]];
                masks[4] = zero_and_all_one[current_control_bits[4]];
                masks[5] = zero_and_all_one[current_control_bits[5]];
                masks[6] = zero_and_all_one[current_control_bits[6]];
                masks[7] = zero_and_all_one[current_control_bits[7]];

                current_seeds[0] = expanded_seeds[0] ^ (masks[0] & key.correction_words[depth + 3].seed);
                current_seeds[1] = expanded_seeds[1] ^ (masks[1] & key.correction_words[depth + 3].seed);
                current_seeds[2] = expanded_seeds[2] ^ (masks[2] & key.correction_words[depth + 3].seed);
                current_seeds[3] = expanded_seeds[3] ^ (masks[3] & key.correction_words[depth + 3].seed);
                current_seeds[4] = expanded_seeds[4] ^ (masks[4] & key.correction_words[depth + 3].seed);
                current_seeds[5] = expanded_seeds[5] ^ (masks[5] & key.correction_words[depth + 3].seed);
                current_seeds[6] = expanded_seeds[6] ^ (masks[6] & key.correction_words[depth + 3].seed);
                current_seeds[7] = expanded_seeds[7] ^ (masks[7] & key.correction_words[depth + 3].seed);

                current_control_bits[0] = expanded_control_bits[0] ^ (current_control_bits[0] & key.correction_words[depth + 3].control_left);
                current_control_bits[1] = expanded_control_bits[1] ^ (current_control_bits[1] & key.correction_words[depth + 3].control_left);
                current_control_bits[2] = expanded_control_bits[2] ^ (current_control_bits[2] & key.correction_words[depth + 3].control_left);
                current_control_bits[3] = expanded_control_bits[3] ^ (current_control_bits[3] & key.correction_words[depth + 3].control_left);
                current_control_bits[4] = expanded_control_bits[4] ^ (current_control_bits[4] & key.correction_words[depth + 3].control_left);
                current_control_bits[5] = expanded_control_bits[5] ^ (current_control_bits[5] & key.correction_words[depth + 3].control_left);
                current_control_bits[6] = expanded_control_bits[6] ^ (current_control_bits[6] & key.correction_words[depth + 3].control_left);
                current_control_bits[7] = expanded_control_bits[7] ^ (current_control_bits[7] & key.correction_words[depth + 3].control_left);
            } else {    // Right
                prg_seed_right.Evaluate(current_seeds, expanded_seeds);

                expanded_control_bits[0] = Lsb(expanded_seeds[0]);
                expanded_control_bits[1] = Lsb(expanded_seeds[1]);
                expanded_control_bits[2] = Lsb(expanded_seeds[2]);
                expanded_control_bits[3] = Lsb(expanded_seeds[3]);
                expanded_control_bits[4] = Lsb(expanded_seeds[4]);
                expanded_control_bits[5] = Lsb(expanded_seeds[5]);
                expanded_control_bits[6] = Lsb(expanded_seeds[6]);
                expanded_control_bits[7] = Lsb(expanded_seeds[7]);

                masks[0] = zero_and_all_one[current_control_bits[0]];
                masks[1] = zero_and_all_one[current_control_bits[1]];
                masks[2] = zero_and_all_one[current_control_bits[2]];
                masks[3] = zero_and_all_one[current_control_bits[3]];
                masks[4] = zero_and_all_one[current_control_bits[4]];
                masks[5] = zero_and_all_one[current_control_bits[5]];
                masks[6] = zero_and_all_one[current_control_bits[6]];
                masks[7] = zero_and_all_one[current_control_bits[7]];

                current_seeds[0] = expanded_seeds[0] ^ (masks[0] & key.correction_words[depth + 3].seed);
                current_seeds[1] = expanded_seeds[1] ^ (masks[1] & key.correction_words[depth + 3].seed);
                current_seeds[2] = expanded_seeds[2] ^ (masks[2] & key.correction_words[depth + 3].seed);
                current_seeds[3] = expanded_seeds[3] ^ (masks[3] & key.correction_words[depth + 3].seed);
                current_seeds[4] = expanded_seeds[4] ^ (masks[4] & key.correction_words[depth + 3].seed);
                current_seeds[5] = expanded_seeds[5] ^ (masks[5] & key.correction_words[depth + 3].seed);
                current_seeds[6] = expanded_seeds[6] ^ (masks[6] & key.correction_words[depth + 3].seed);
                current_seeds[7] = expanded_seeds[7] ^ (masks[7] & key.correction_words[depth + 3].seed);

                current_control_bits[0] = expanded_control_bits[0] ^ (current_control_bits[0] & key.correction_words[depth + 3].control_right);
                current_control_bits[1] = expanded_control_bits[1] ^ (current_control_bits[1] & key.correction_words[depth + 3].control_right);
                current_control_bits[2] = expanded_control_bits[2] ^ (current_control_bits[2] & key.correction_words[depth + 3].control_right);
                current_control_bits[3] = expanded_control_bits[3] ^ (current_control_bits[3] & key.correction_words[depth + 3].control_right);
                current_control_bits[4] = expanded_control_bits[4] ^ (current_control_bits[4] & key.correction_words[depth + 3].control_right);
                current_control_bits[5] = expanded_control_bits[5] ^ (current_control_bits[5] & key.correction_words[depth + 3].control_right);
                current_control_bits[6] = expanded_control_bits[6] ^ (current_control_bits[6] & key.correction_words[depth + 3].control_right);
                current_control_bits[7] = expanded_control_bits[7] ^ (current_control_bits[7] & key.correction_words[depth + 3].control_right);
            }
            depth++;
            prev_seeds[depth][0] = current_seeds[0];
            prev_seeds[depth][1] = current_seeds[1];
            prev_seeds[depth][2] = current_seeds[2];
            prev_seeds[depth][3] = current_seeds[3];
            prev_seeds[depth][4] = current_seeds[4];
            prev_seeds[depth][5] = current_seeds[5];
            prev_seeds[depth][6] = current_seeds[6];
            prev_seeds[depth][7] = current_seeds[7];

            prev_control_bits[depth][0] = current_control_bits[0];
            prev_control_bits[depth][1] = current_control_bits[1];
            prev_control_bits[depth][2] = current_control_bits[2];
            prev_control_bits[depth][3] = current_control_bits[3];
            prev_control_bits[depth][4] = current_control_bits[4];
            prev_control_bits[depth][5] = current_control_bits[5];
            prev_control_bits[depth][6] = current_control_bits[6];
            prev_control_bits[depth][7] = current_control_bits[7];
        }

        masks[0] = zero_and_all_one[current_control_bits[0]];
        masks[1] = zero_and_all_one[current_control_bits[1]];
        masks[2] = zero_and_all_one[current_control_bits[2]];
        masks[3] = zero_and_all_one[current_control_bits[3]];
        masks[4] = zero_and_all_one[current_control_bits[4]];
        masks[5] = zero_and_all_one[current_control_bits[5]];
        masks[6] = zero_and_all_one[current_control_bits[6]];
        masks[7] = zero_and_all_one[current_control_bits[7]];

        std::array<Block, 8> output_blocks = {zero_block, zero_block, zero_block, zero_block, zero_block, zero_block, zero_block, zero_block};

        output_blocks[0] = current_seeds[0] ^ (masks[0] & key.output);
        output_blocks[1] = current_seeds[1] ^ (masks[1] & key.output);
        output_blocks[2] = current_seeds[2] ^ (masks[2] & key.output);
        output_blocks[3] = current_seeds[3] ^ (masks[3] & key.output);
        output_blocks[4] = current_seeds[4] ^ (masks[4] & key.output);
        output_blocks[5] = current_seeds[5] ^ (masks[5] & key.output);
        output_blocks[6] = current_seeds[6] ^ (masks[6] & key.output);
        output_blocks[7] = current_seeds[7] ^ (masks[7] & key.output);

        output_vec[idx][0] = output_blocks[0];
        output_vec[idx][1] = output_blocks[1];
        output_vec[idx][2] = output_blocks[2];
        output_vec[idx][3] = output_blocks[3];
        output_vec[idx][4] = output_blocks[4];
        output_vec[idx][5] = output_blocks[5];
        output_vec[idx][6] = output_blocks[6];
        output_vec[idx][7] = output_blocks[7];

        int shift = (idx + 1U) ^ idx;
        depth -= static_cast<int>(std::floor(std::log2(shift))) + 1;
        idx++;
    }

    for (uint32_t i = 0; i < end; i++) {
        for (uint32_t j = 0; j < 8; j++) {
            std::vector<uint32_t> output = output_vec[i][j].ConvertVec(128, e);
            uint32_t              start  = j * utils::Pow(2, n - 3) + i * 128;
            for (uint32_t k = start; k < start + 128; k++) {
                outputs[k] = output[k - start];
            }
        }
    }
}

void DistributedPointFunction::FullDomainRecursive(const DpfKey &key, std::vector<uint32_t> &outputs) const {
    int nu = this->params_.terminate_bitsize;
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate full domain with early termination"), this->params_.debug);
#endif

    // Get the seed and control bit from the DPF key.
    Block seed        = key.init_seed;
    bool  control_bit = key.party_id != 0;

    Traverse(seed, control_bit, key, nu, 0, outputs);
}

std::pair<DpfKey, DpfKey> DistributedPointFunction::GenerateKeysNaive(const uint32_t alpha, const uint32_t beta) const {
    uint32_t n = this->params_.input_bitsize;
    uint32_t e = this->params_.element_bitsize;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Generate DPF keys (naive)"), debug);
#endif

    // Create DPF keys for both parties.
    std::array<DpfKey, 2> keys;    // keys[party id]
    keys[0].Initialize(this->params_, 0, true);
    keys[1].Initialize(this->params_, 1, true);

    // line 2-3: initial seed and control bit
    // Set initial seeds and control bit
    std::array<Block, 2> seeds;                 // seeds[party id]
    std::array<bool, 2>  control_bits{0, 1};    // control_bits[party id]
    // Set initial seeds for both parties.
    seeds[0].SetRandom();
    seeds[1].SetRandom();
    keys[0].init_seed = seeds[0];
    keys[1].init_seed = seeds[1];

#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Set initial seed and control bit", debug);
    seeds[0].PrintBlockHexTrace(LOCATION, "ID=" + std::to_string(0) + " Initial seed: ", debug);
    seeds[1].PrintBlockHexTrace(LOCATION, "ID=" + std::to_string(1) + " Initial seed: ", debug);
    utils::Logger::TraceLog(LOCATION, "ID=" + std::to_string(0) + " Control bit: " + std::to_string(control_bits[0]), debug);
    utils::Logger::TraceLog(LOCATION, "ID=" + std::to_string(1) + " Control bit: " + std::to_string(control_bits[1]), debug);
#endif

    // line 5-13: Generate next seed and compute correction words
    for (uint32_t i = 0; i < n; i++) {
        bool current_bit = (alpha & (1 << (n - i - 1))) != 0;
        GenerateNextSeed(i, current_bit, keys, seeds, control_bits);
    }

    // Calculate and set the output for both keys.
    Block output   = Block(0, utils::Mod(utils::Pow(-1, control_bits[1]) * (beta - seeds[0].Convert(e) + seeds[1].Convert(e)), e));
    keys[0].output = output;
    keys[1].output = output;

#ifdef LOG_LEVEL_TRACE
    // Log the generated keys and output.
    utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(output.Convert(e)), debug);
    utils::AddNewLine(debug);
    keys[0].PrintDpfKey(params_, debug, true);
    utils::AddNewLine(debug);
    keys[1].PrintDpfKey(params_, debug, true);
    utils::AddNewLine(debug);
#endif

    // Return the generated keys as a pair.
    return std::make_pair(std::move(keys[0]), std::move(keys[1]));
}

uint32_t DistributedPointFunction::EvaluateAtNaive(const DpfKey &key, const uint32_t x) const {
    uint32_t n = this->params_.input_bitsize;
    uint32_t e = this->params_.element_bitsize;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate input with DPF key"), debug);
    utils::Logger::TraceLog(LOCATION, "Party ID: " + std::to_string(key.party_id), debug);
#endif

    // Get the seed and control bit from the DPF key.
    Block seed        = key.init_seed;
    bool  control_bit = key.party_id != 0;

    // line 3-8: definition
    std::array<Block, 2> expanded_seeds;           // expanded_seeds[keep or lose]
    std::array<bool, 2>  expanded_control_bits;    // expanded_control_bits[keep or lose]

    // Iterate over each level of the DPF protocol.
    for (uint32_t i = 0; i < n; i++) {
        std::string current_level = "|Level=" + std::to_string(i) + "| ";

        EvaluateNextSeed(
            i, key.correction_words[i],
            seed, control_bit,
            expanded_seeds, expanded_control_bits);

        // line 6: Update the seed and control bit based on the current input bit.
        bool current_bit = (x & (1 << (n - i - 1))) != 0;

        if (current_bit) {    // current bit = 1
            seed        = expanded_seeds[kRight];
            control_bit = expanded_control_bits[kRight];
        } else {    // current bit = 0
            seed        = expanded_seeds[kLeft];
            control_bit = expanded_control_bits[kLeft];
        }

#ifdef LOG_LEVEL_TRACE
        utils::Logger::TraceLog(LOCATION, current_level + "Current bit: " + std::to_string(current_bit), debug);
        seed.PrintBlockHexTrace(LOCATION, current_level + "Next seed: ", debug);
        utils::Logger::TraceLog(LOCATION, current_level + "Next control bit: " + std::to_string(control_bit), debug);
#endif
    }

    // Calculate the final output based on the DPF protocol.
    Block    output_block = ComputeOutputBlock(seed, control_bit, key);
    uint32_t output       = output_block.Convert(e);
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(output), debug);
#endif
    return output;
}

void DistributedPointFunction::FullDomainNaiveNaive(const DpfKey &key, std::vector<uint32_t> &outputs) const {
    int n = this->params_.input_bitsize;
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate full domain naive"), this->params_.debug);
#endif

    for (uint32_t i = 0; i < utils::Pow(2, n); i++) {
        outputs[i] = EvaluateAtNaive(key, i);
    }
}

void DistributedPointFunction::GenerateNextSeed(
    const uint32_t current_tree_level, const bool current_bit,
    const std::array<DpfKey, 2> &keys, std::array<Block, 2> &current_seeds, std::array<bool, 2> &current_control_bits) const {

#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::AddNewLine(debug);
    std::string current_level = "|Level=" + std::to_string(current_tree_level) + "| ";
#endif

    std::array<std::array<Block, 2>, 2> expanded_seeds;           // expanded_seeds[party id][keep or lose]
    std::array<std::array<bool, 2>, 2>  expanded_control_bits;    // expanded_control_bits[party id][keep or lose]
    CorrectionWord                      correction_word;
    Block                               seed_correction;
    std::array<bool, 2>                 control_bit_correction;    // control_bit_correction[keep or lose]

    // line 5: expand seeds
    for (int j = 0; j < 2; j++) {
        prg_seed_left.Evaluate(current_seeds[j], expanded_seeds[j][kLeft]);
        prg_seed_right.Evaluate(current_seeds[j], expanded_seeds[j][kRight]);
        expanded_control_bits[j][kLeft]  = Lsb(expanded_seeds[j][kLeft]);
        expanded_control_bits[j][kRight] = Lsb(expanded_seeds[j][kRight]);
    }
#ifdef LOG_LEVEL_TRACE
    for (int j = 0; j < 2; j++) {
        expanded_seeds[j][kLeft].PrintBlockHexTrace(LOCATION, current_level + "ID=" + std::to_string(j) + " Expanded seed (L): ", debug);
        expanded_seeds[j][kRight].PrintBlockHexTrace(LOCATION, current_level + "ID=" + std::to_string(j) + " Expanded seed (R): ", debug);
        utils::Logger::TraceLog(LOCATION, current_level + "ID=" + std::to_string(j) + " Expanded control bit (L): " + std::to_string(expanded_control_bits[j][kLeft]) + ", (R): " + std::to_string(expanded_control_bits[j][kRight]), debug);
    }
#endif

    // line 6-8: keep or lose
    bool keep = current_bit, lose = !current_bit;
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, current_level + "Current bit: " + std::to_string(current_bit) + " (Keep: " + std::to_string(keep) + ", Lose: " + std::to_string(lose) + ")", debug);
#endif

    // line 9: seed correction
    seed_correction = expanded_seeds[0][lose] ^ expanded_seeds[1][lose];
#ifdef LOG_LEVEL_TRACE
    seed_correction.PrintBlockHexTrace(LOCATION, current_level + "Correction seed: ", debug);
#endif

    // line10: control bit correction
    control_bit_correction[kLeft]  = expanded_control_bits[0][kLeft] ^ expanded_control_bits[1][kLeft] ^ current_bit ^ 1;
    control_bit_correction[kRight] = expanded_control_bits[0][kRight] ^ expanded_control_bits[1][kRight] ^ current_bit;
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, current_level + "Correction control bit (L): " + std::to_string(control_bit_correction[kLeft]) + ", (R): " + std::to_string(control_bit_correction[kRight]), debug);
#endif

    // line 11: set correction word
    correction_word.seed                         = seed_correction;
    correction_word.control_left                 = control_bit_correction[kLeft];
    correction_word.control_right                = control_bit_correction[kRight];
    keys[0].correction_words[current_tree_level] = correction_word;
    keys[1].correction_words[current_tree_level] = correction_word;

    // line 12-13: update seed and control bits
    for (int j = 0; j < 2; j++) {
        current_seeds[j] = expanded_seeds[j][keep];
        if (current_control_bits[j]) {
            current_seeds[j] = current_seeds[j] ^ seed_correction;
        }
        current_control_bits[j] = expanded_control_bits[j][keep] ^ (current_control_bits[j] & control_bit_correction[keep]);

#ifdef LOG_LEVEL_TRACE
        current_seeds[j].PrintBlockHexTrace(LOCATION, current_level + "ID=" + std::to_string(j) + " Updated seed: ", debug);
        utils::Logger::TraceLog(LOCATION, current_level + "ID=" + std::to_string(j) + " Control bit: " + std::to_string(current_control_bits[j]), debug);
#endif
    }
}

void DistributedPointFunction::EvaluateNextSeed(
    const uint32_t current_tree_level, const CorrectionWord &correction_word,
    const Block &current_seed, const bool current_control_bit,
    std::array<Block, 2> &expanded_seeds, std::array<bool, 2> &expanded_control_bits) const {

    // line 4-5: expand the seeds using pseudorandom generators
    prg_seed_left.Evaluate(current_seed, expanded_seeds[kLeft]);
    prg_seed_right.Evaluate(current_seed, expanded_seeds[kRight]);
    expanded_control_bits[kLeft]  = Lsb(expanded_seeds[kLeft]);
    expanded_control_bits[kRight] = Lsb(expanded_seeds[kRight]);

#ifdef LOG_LEVEL_TRACE
    bool        debug         = this->params_.debug;
    std::string current_level = "|Level=" + std::to_string(current_tree_level) + "| ";
    current_seed.PrintBlockHexTrace(LOCATION, current_level + "Current seed: ", debug);
    utils::Logger::TraceLog(LOCATION, current_level + "Control bit: " + std::to_string(current_control_bit), debug);
    expanded_seeds[kLeft].PrintBlockHexTrace(LOCATION, current_level + "Expanded seed (L): ", debug);
    expanded_seeds[kRight].PrintBlockHexTrace(LOCATION, current_level + "Expanded seed (R): ", debug);
    utils::Logger::TraceLog(LOCATION, current_level + "Expanded control bit (L): " + std::to_string(expanded_control_bits[kLeft]) + ", (R): " + std::to_string(expanded_control_bits[kRight]), debug);
#endif

    // Apply correction word if control bit is set.
    if (current_control_bit) {
        expanded_seeds[kLeft]  = expanded_seeds[kLeft] ^ correction_word.seed;
        expanded_seeds[kRight] = expanded_seeds[kRight] ^ correction_word.seed;
        expanded_control_bits[kLeft] ^= correction_word.control_left;
        expanded_control_bits[kRight] ^= correction_word.control_right;
    }
}

void DistributedPointFunction::Traverse(const Block &current_seed, const bool current_control_bit, const DpfKey &key, uint32_t i, uint32_t j, std::vector<uint32_t> &outputs) const {
    uint32_t n          = this->params_.input_bitsize;
    uint32_t e          = this->params_.element_bitsize;
    uint32_t nu         = this->params_.terminate_bitsize;
    uint32_t term_nodes = utils::Pow(2, n - nu);

    if (i > 0) {
        // line 3-8: definition
        std::array<Block, 2> expanded_seeds;           // expanded_seeds[keep or lose]
        std::array<bool, 2>  expanded_control_bits;    // expanded_control_bits[keep or lose]

        EvaluateNextSeed(nu - i, key.correction_words[nu - i], current_seed, current_control_bit, expanded_seeds, expanded_control_bits);

        Traverse(expanded_seeds[kLeft], expanded_control_bits[kLeft], key, i - 1, j, outputs);
        Traverse(expanded_seeds[kRight], expanded_control_bits[kRight], key, i - 1, j + utils::Pow(2, n - nu + i - 1), outputs);
    } else {    // i = 0
        Block                 output_block = ComputeOutputBlock(current_seed, current_control_bit, key);
        std::vector<uint32_t> output_vec   = output_block.ConvertVec(term_nodes, e);
        for (uint32_t k = j; k < j + utils::Pow(2, n - nu); k++) {
            outputs[k] = output_vec[k - j];
        }
    }
}

void DistributedPointFunction::SetKeyOutput(const uint32_t alpha, const uint32_t beta, const bool control_bit, const std::array<Block, 2> &seeds, std::array<DpfKey, 2> &keys) const {
    uint32_t alpha_hat = utils::GetLowerNBits(alpha, this->params_.input_bitsize - this->params_.terminate_bitsize);
    uint32_t num       = utils::Pow(2, this->params_.input_bitsize - this->params_.terminate_bitsize);
    Block    beta_block(0, beta);
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Alpha: " + std::to_string(alpha) + ", Alpha hat: " + std::to_string(alpha_hat) + ", Beta: " + std::to_string(beta) + ", num: " + std::to_string(num), this->params_.debug);
    beta_block.PrintBlockBinTrace(LOCATION, "(Before) Beta block: ", this->params_.debug);
#endif
    // Shift the beta block based on the alpha hat.
    const uint8_t shift = kSecurityParameter / num * alpha_hat;
    if (shift >= 64) {
        beta_block = _mm_slli_si128(beta_block, 8);
        beta_block = beta_block << (shift - 64);
    } else {
        // Because beta is 32 bits or less, the shift of the upper bits is ignored.
        beta_block = beta_block << shift;
    }
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Shift amount: " + std::to_string(shift), this->params_.debug);
    beta_block.PrintBlockBinTrace(LOCATION, "(Update) Beta block: ", this->params_.debug);
#endif

    Block output;
    if (num == 4) {
        if (control_bit) {
            output         = _mm_sub_epi32(zero_block, _mm_add_epi32(_mm_sub_epi32(beta_block, seeds[0]), seeds[1]));
            keys[0].output = output;
            keys[1].output = output;
        } else {
            output         = _mm_add_epi32(_mm_sub_epi32(beta_block, seeds[0]), seeds[1]);
            keys[0].output = output;
            keys[1].output = output;
        }
    } else if (num == 8) {
        if (control_bit) {
            output         = _mm_sub_epi16(zero_block, _mm_add_epi16(_mm_sub_epi16(beta_block, seeds[0]), seeds[1]));
            keys[0].output = output;
            keys[1].output = output;
        } else {
            output         = _mm_add_epi16(_mm_sub_epi16(beta_block, seeds[0]), seeds[1]);
            keys[0].output = output;
            keys[1].output = output;
        }
    } else if (num == 16) {
        if (control_bit) {
            output         = _mm_sub_epi8(zero_block, _mm_add_epi8(_mm_sub_epi8(beta_block, seeds[0]), seeds[1]));
            keys[0].output = output;
            keys[1].output = output;
        } else {
            output         = _mm_add_epi8(_mm_sub_epi8(beta_block, seeds[0]), seeds[1]);
            keys[0].output = output;
            keys[1].output = output;
        }
    } else if (num == 32) {
        std::vector<uint32_t> beta_vec(num);
        std::vector<uint32_t> seeds_vec[2];
        seeds_vec[0]  = seeds[0].ConvertVec(num, 4);
        seeds_vec[1]  = seeds[1].ConvertVec(num, 4);
        uint32_t cond = (control_bit % 2 == 0) ? 1 : -1;
        for (uint32_t i = 0; i < beta_vec.size(); ++i) {
            if (i == alpha_hat) {
                beta_vec[i] = cond * (beta - seeds_vec[0][i] + seeds_vec[1][i]);
            } else {
                beta_vec[i] = cond * (-seeds_vec[0][i] + seeds_vec[1][i]);
            }
        }
        output.FromVec(beta_vec, num, 4);
        keys[0].output = output;
        keys[1].output = output;
    } else if (num == 64) {
        std::vector<uint32_t> beta_vec(num);
        std::vector<uint32_t> seeds_vec[2];
        seeds_vec[0]  = seeds[0].ConvertVec(num, 2);
        seeds_vec[1]  = seeds[1].ConvertVec(num, 2);
        uint32_t cond = (control_bit % 2 == 0) ? 1 : -1;
        for (uint32_t i = 0; i < beta_vec.size(); ++i) {
            if (i == alpha_hat) {
                beta_vec[i] = cond * (beta - seeds_vec[0][i] + seeds_vec[1][i]);
            } else {
                beta_vec[i] = cond * (-seeds_vec[0][i] + seeds_vec[1][i]);
            }
        }
        output.FromVec(beta_vec, num, 2);
        keys[0].output = output;
        keys[1].output = output;
    } else if (num == 128) {
        output         = beta_block ^ seeds[0] ^ seeds[1];
        keys[0].output = output;
        keys[1].output = output;
    }
}

Block DistributedPointFunction::ComputeOutputBlock(const Block &current_seed, const bool current_control_bit, const DpfKey &key) const {
    Block    mask = zero_and_all_one[current_control_bit];
    uint32_t num  = utils::Pow(2, this->params_.input_bitsize - this->params_.terminate_bitsize);

    Block output = zero_block;
    if (num == 4) {
        if (key.party_id) {
            output = _mm_sub_epi32(zero_block, _mm_add_epi32(current_seed, mask & key.output));
        } else {
            output = _mm_add_epi32(current_seed, mask & key.output);
        }
    } else if (num == 8) {
        if (key.party_id) {
            output = _mm_sub_epi16(zero_block, _mm_add_epi16(current_seed, mask & key.output));
        } else {
            output = _mm_add_epi16(current_seed, mask & key.output);
        }
    } else if (num == 16) {
        if (key.party_id) {
            output = _mm_sub_epi8(zero_block, _mm_add_epi8(current_seed, mask & key.output));
        } else {
            output = _mm_add_epi8(current_seed, mask & key.output);
        }
    } else if (num == 32) {
        std::vector<uint32_t> out_vec(num);
        std::vector<uint32_t> seed_vec = current_seed.ConvertVec(num, 4);
        std::vector<uint32_t> key_vec  = key.output.ConvertVec(num, 4);
        uint32_t              cond     = (key.party_id % 2 == 0) ? 1 : -1;
        for (uint32_t i = 0; i < out_vec.size(); ++i) {
            out_vec[i] = cond * (seed_vec[i] + (current_control_bit * key_vec[i]));
        }
        output.FromVec(out_vec, num, 4);
    } else if (num == 64) {
        std::vector<uint32_t> out_vec(num);
        std::vector<uint32_t> seed_vec = current_seed.ConvertVec(num, 2);
        std::vector<uint32_t> key_vec  = key.output.ConvertVec(num, 2);
        uint32_t              cond     = (key.party_id % 2 == 0) ? 1 : -1;
        for (uint32_t i = 0; i < out_vec.size(); ++i) {
            out_vec[i] = cond * (seed_vec[i] + (current_control_bit * key_vec[i]));
        }
        output.FromVec(out_vec, num, 2);
    } else if (num == 128) {
        output = current_seed ^ (mask & key.output);
    }
    return output;
}

}    // namespace dpf
}    // namespace fss
