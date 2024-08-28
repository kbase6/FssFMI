/**
 * @file distributed_comparison_function.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-02-05
 * @copyright Copyright (c) 2024
 * @brief Distributed Comparison Function (DCF) implementation.
 */

#include "distributed_comparison_function.hpp"

#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"
#include "../prg/prg.hpp"

namespace {

// Pseudorandom generators for various key values
static const fss::prg::PRG prg_seed_left   = fss::prg::PRG::Create(fss::kPrgKeySeedLeft);
static const fss::prg::PRG prg_seed_right  = fss::prg::PRG::Create(fss::kPrgKeySeedRight);
static const fss::prg::PRG prg_value_left  = fss::prg::PRG::Create(fss::kPrgKeyValueLeft);
static const fss::prg::PRG prg_value_right = fss::prg::PRG::Create(fss::kPrgKeyValueRight);

}    // namespace

namespace fss {
namespace dcf {

DcfParameters::DcfParameters()
    : input_bitsize(0), element_bitsize(0), debug(false) {
}

DcfParameters::DcfParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info)
    : input_bitsize(n), element_bitsize(e), debug(dbg_info.dcf_debug) {
}

void DcfKey::Initialize(const uint32_t n, const uint32_t party_id) {
    this->party_id         = party_id;
    this->cw_length        = n;
    this->correction_words = new CorrectionWord[n];
    this->output           = 0;
}

void DcfKey::PrintDcfKey(const bool debug) const {
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("DCF Key"), debug);
    utils::Logger::TraceLog(LOCATION, "Party ID: " + std::to_string(this->party_id), debug);
    this->init_seed.PrintBlockHexTrace(LOCATION, "Initial seed: ", debug);
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Correction words"), debug);
    for (uint32_t i = 0; i < this->cw_length; i++) {
        this->correction_words[i].seed.PrintBlockHexTrace(LOCATION, "Level(" + std::to_string(i) + ") Seed -> ", debug);
        utils::Logger::TraceLog(LOCATION, "Level(" + std::to_string(i) + ") Control bit -> (L):" + std::to_string(this->correction_words[i].control_left) + ", (R): " + std::to_string(this->correction_words[i].control_right), debug);
        utils::Logger::TraceLog(LOCATION, "Level(" + std::to_string(i) + ") Value -> " + std::to_string(this->correction_words[i].value), debug);
    }
    utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(this->output), debug);
    utils::Logger::TraceLog(LOCATION, utils::kDash, debug);
#endif
}

void DcfKey::FreeDcfKey() {
    delete[] this->correction_words;
}

CorrectionWord::CorrectionWord()
    : seed(Block(zero_block)), control_left(false), control_right(false), value(0) {
}

DistributedComparisonFunction::DistributedComparisonFunction(const DcfParameters params)
    : params_(params) {
}

std::pair<DcfKey, DcfKey> DistributedComparisonFunction::GenerateKeys(const uint32_t alpha, const uint32_t beta) const {
    uint32_t n = this->params_.input_bitsize;
    uint32_t e = this->params_.element_bitsize;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Generate DCF keys"), debug);
    utils::Logger::TraceLog(LOCATION, "(input size, element size) = (" + std::to_string(n) + ", " + std::to_string(e) + ")", debug);
    utils::Logger::TraceLog(LOCATION, "(alpha, beta) = (" + std::to_string(alpha) + ", " + std::to_string(beta) + ")", debug);
#endif

    std::array<DcfKey, 2> keys;    // keys[party id]
    keys[0].Initialize(n, 0);
    keys[1].Initialize(n, 1);

    // line 2-3: initial seed, value and control bit
    std::array<Block, 2> seeds;                 // seeds[party id]
    std::array<bool, 2>  control_bits{0, 1};    // control_bits[party id]
    uint32_t             value = 0;
    seeds[0].SetRandom();
    seeds[1].SetRandom();
    keys[0].init_seed = seeds[0];
    keys[1].init_seed = seeds[1];

#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Set initial seed, value and control bit", debug);
    seeds[0].PrintBlockHexTrace(LOCATION, "ID=" + std::to_string(0) + " Initial seed: ", debug);
    seeds[1].PrintBlockHexTrace(LOCATION, "ID=" + std::to_string(1) + " Initial seed: ", debug);
    utils::Logger::TraceLog(LOCATION, "ID=" + std::to_string(0) + " Control bit: " + std::to_string(control_bits[0]), debug);
    utils::Logger::TraceLog(LOCATION, "ID=" + std::to_string(1) + " Control bit: " + std::to_string(control_bits[1]), debug);
    utils::Logger::TraceLog(LOCATION, "Initial value: " + std::to_string(value), debug);
#endif

    // line 5-18: definition
    std::array<std::array<Block, 2>, 2> expanded_seeds;           // expanded_seeds[party id][keep or lose]
    std::array<std::array<bool, 2>, 2>  expanded_control_bits;    // expanded_control_bits[party id][keep or lose]
    std::array<std::array<Block, 2>, 2> expanded_values;          // expanded_values[party id][keep or lose]
    CorrectionWord                      correction_word;
    Block                               seed_correction;
    std::array<bool, 2>                 control_bit_correction;    // control_bit_correction[keep or lose]
    uint32_t                            value_correction;
    for (uint32_t i = 0; i < n; i++) {
#ifdef LOG_LEVEL_TRACE
        utils::AddNewLine(debug);
        std::string current_level = "|Level=" + std::to_string(i) + "| ";
#endif

        // line 5-6: expand seeds
        for (int j = 0; j < 2; j++) {
            prg_seed_left.Evaluate(seeds[j], expanded_seeds[j][kLeft]);
            prg_seed_right.Evaluate(seeds[j], expanded_seeds[j][kRight]);
            expanded_control_bits[j][kLeft]  = Lsb(expanded_seeds[j][kLeft]);
            expanded_control_bits[j][kRight] = Lsb(expanded_seeds[j][kRight]);
            prg_value_left.Evaluate(seeds[j], expanded_values[j][kLeft]);
            prg_value_right.Evaluate(seeds[j], expanded_values[j][kRight]);
        }

#ifdef LOG_LEVEL_TRACE
        utils::AddNewLine(debug);
        for (int j = 0; j < 2; j++) {
            expanded_seeds[j][kLeft].PrintBlockHexTrace(LOCATION, current_level + "ID=" + std::to_string(j) + " Expanded seed (L): ", debug);
            expanded_seeds[j][kRight].PrintBlockHexTrace(LOCATION, current_level + "ID=" + std::to_string(j) + " Expanded seed (R): ", debug);
            utils::Logger::TraceLog(LOCATION, current_level + "ID=" + std::to_string(j) + " Expanded control bit (L): " + std::to_string(expanded_control_bits[j][kLeft]) + ", (R): " + std::to_string(expanded_control_bits[j][kRight]), debug);
            expanded_values[j][kLeft].PrintBlockHexTrace(LOCATION, current_level + "ID=" + std::to_string(j) + " Expanded value (L): ", debug);
            expanded_values[j][kRight].PrintBlockHexTrace(LOCATION, current_level + "ID=" + std::to_string(j) + " Expanded value (R): ", debug);
        }
#endif

        // line 7-9: keep or lose
        bool current_bit = (alpha & (1 << (n - i - 1))) != 0;
        bool keep = current_bit, lose = !current_bit;
        // line 10: seed correction
        seed_correction = expanded_seeds[0][lose] ^ expanded_seeds[1][lose];

#ifdef LOG_LEVEL_TRACE
        utils::Logger::TraceLog(LOCATION, "Current bit: " + std::to_string(current_bit), debug);
        utils::Logger::TraceLog(LOCATION, "Keep: " + std::to_string(keep) + ", Lose: " + std::to_string(lose), debug);
        seed_correction.PrintBlockHexTrace(LOCATION, current_level + "Seed correction: ", debug);
#endif

        // line 11-13: value correction
        value_correction = utils::Pow(-1, control_bits[1]) * (expanded_values[1][lose].Convert(e) - expanded_values[0][lose].Convert(e) - value);
        value_correction = utils::Mod(value_correction, e);
        if (lose == kLeft) {
#ifdef LOG_LEVEL_TRACE
            utils::Logger::TraceLog(LOCATION, "(Lose=L)", debug);
#endif
            value_correction += utils::Pow(-1, control_bits[1]) * beta;
            value_correction = utils::Mod(value_correction, e);
        }

        // line 15: control bit correction
        control_bit_correction[kLeft]  = expanded_control_bits[0][kLeft] ^ expanded_control_bits[1][kLeft] ^ current_bit ^ 1;
        control_bit_correction[kRight] = expanded_control_bits[0][kRight] ^ expanded_control_bits[1][kRight] ^ current_bit;
#ifdef LOG_LEVEL_TRACE
        utils::Logger::TraceLog(LOCATION, current_level + "Value correction: " + std::to_string(value_correction), debug);
        utils::Logger::TraceLog(LOCATION, current_level + "Control bit correction (L): " + std::to_string(control_bit_correction[kLeft]) + ", (R): " + std::to_string(control_bit_correction[kRight]), debug);
#endif

        // line 16: set correction word
        correction_word.seed          = seed_correction;
        correction_word.control_left  = control_bit_correction[kLeft];
        correction_word.control_right = control_bit_correction[kRight];
        correction_word.value         = value_correction;
        keys[0].correction_words[i]   = correction_word;
        keys[1].correction_words[i]   = correction_word;

        // line 14: update value
        value = value - expanded_values[1][keep].Convert(e) + expanded_values[0][keep].Convert(e) + (utils::Pow(-1, control_bits[1]) * value_correction);
        value = utils::Mod(value, e);
#ifdef LOG_LEVEL_TRACE
        utils::Logger::TraceLog(LOCATION, current_level + "Updated value: " + std::to_string(value), debug);
#endif

        // line 17-18: update seed and control bits
        for (int j = 0; j < 2; j++) {
            seeds[j] = expanded_seeds[j][keep];
            if (control_bits[j]) {
                seeds[j] = seeds[j] ^ seed_correction;
            }
            control_bits[j] = expanded_control_bits[j][keep] ^ (control_bits[j] & control_bit_correction[keep]);
#ifdef LOG_LEVEL_TRACE
            seeds[j].PrintBlockHexTrace(LOCATION, current_level + "ID=" + std::to_string(j) + " Updated seed: ", debug);
            utils::Logger::TraceLog(LOCATION, current_level + "ID=" + std::to_string(j) + " Control bit: " + std::to_string(control_bits[j]), debug);
#endif
        }
    }
    uint32_t output = utils::Pow(-1, control_bits[1]) * (seeds[1].Convert(e) - seeds[0].Convert(e) - value);
    output          = utils::Mod(output, e);
    keys[0].output  = output;
    keys[1].output  = output;

#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(output), debug);
    utils::AddNewLine(debug);
    keys[0].PrintDcfKey(debug);
    utils::AddNewLine(debug);
    keys[1].PrintDcfKey(debug);
    utils::AddNewLine(debug);
#endif

    // Return the generated keys as a pair.
    return std::make_pair(std::move(keys[0]), std::move(keys[1]));
}

uint32_t DistributedComparisonFunction::EvaluateAt(const DcfKey &key, const uint32_t x) const {
    uint32_t n = this->params_.input_bitsize;
    uint32_t e = this->params_.element_bitsize;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate input with DCF key"), debug);
    utils::Logger::TraceLog(LOCATION, "Party ID: " + std::to_string(key.party_id), debug);
#endif

    // line 1: setup
    Block    seed        = key.init_seed;
    bool     control_bit = key.party_id != 0;
    uint32_t value       = 0;

    // line 3-11: definition
    std::array<Block, 2> expanded_seeds;           // expanded_seeds[keep or lose]
    std::array<bool, 2>  expanded_control_bits;    // expanded_control_bits[keep or lose]
    std::array<Block, 2> expanded_values;          // expanded_values[keep or lose]

    for (uint32_t i = 0; i < n; i++) {
        std::string current_level = "|Level=" + std::to_string(i) + "| ";

        EvaluateNextSeed(
            i, key.correction_words[i],
            seed, control_bit,
            expanded_seeds, expanded_values, expanded_control_bits);

        // line 7: update seed
        bool current_bit = (x & (1 << (n - i - 1))) != 0;

        if (current_bit) {    // current bit = 1
            value += utils::Pow(-1, key.party_id) * (expanded_values[kRight].Convert(e) + (control_bit * key.correction_words[i].value));
            value       = utils::Mod(value, e);
            seed        = expanded_seeds[kRight];
            control_bit = expanded_control_bits[kRight];
        } else {    // current bit = 0
            value += utils::Pow(-1, key.party_id) * (expanded_values[kLeft].Convert(e) + (control_bit * key.correction_words[i].value));
            value       = utils::Mod(value, e);
            seed        = expanded_seeds[kLeft];
            control_bit = expanded_control_bits[kLeft];
        }

#ifdef LOG_LEVEL_TRACE
        utils::Logger::TraceLog(LOCATION, current_level + "Current bit: " + std::to_string(current_bit), debug);
        seed.PrintBlockHexTrace(LOCATION, current_level + "Next seed: ", debug);
        utils::Logger::TraceLog(LOCATION, current_level + "Next control bit: " + std::to_string(control_bit), debug);
#endif
    }

    // Calculate the final output based on the DCF protocol.
    uint32_t output = value + (utils::Pow(-1, key.party_id) * (seed.Convert(e) + (control_bit * key.output)));
    output          = utils::Mod(output, e);

// Log the evaluation result and return the output.
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(output), debug);
#endif
    return output;
}

void DistributedComparisonFunction::EvaluateNextSeed(
    const uint32_t current_tree_level, const CorrectionWord &correction_word,
    const Block &current_seed, const bool current_control_bit,
    std::array<Block, 2> &expanded_seeds, std::array<Block, 2> &expanded_values, std::array<bool, 2> &expanded_control_bits) const {

#ifdef LOG_LEVEL_TRACE
    bool        debug         = this->params_.debug;
    std::string current_level = "|Level=" + std::to_string(current_tree_level) + "| ";
#endif

    // line 4-6: expand seeds
    prg_seed_left.Evaluate(current_seed, expanded_seeds[kLeft]);
    prg_seed_right.Evaluate(current_seed, expanded_seeds[kRight]);
    prg_value_left.Evaluate(current_seed, expanded_values[kLeft]);
    prg_value_right.Evaluate(current_seed, expanded_values[kRight]);
    expanded_control_bits[kLeft]  = Lsb(expanded_seeds[kLeft]);
    expanded_control_bits[kRight] = Lsb(expanded_seeds[kRight]);

#ifdef LOG_LEVEL_TRACE
    current_seed.PrintBlockHexTrace(LOCATION, current_level + "Current seed: ", debug);
    utils::Logger::TraceLog(LOCATION, current_level + "Control bit: " + std::to_string(current_control_bit), debug);
    expanded_seeds[kLeft].PrintBlockHexTrace(LOCATION, current_level + "Expanded Seed (L): ", debug);
    expanded_seeds[kRight].PrintBlockHexTrace(LOCATION, current_level + "Expanded Seed (R): ", debug);
    utils::Logger::TraceLog(LOCATION, current_level + "Expanded control bit (L): " + std::to_string(expanded_control_bits[kLeft]), debug);
    utils::Logger::TraceLog(LOCATION, current_level + "Expanded control bit (R): " + std::to_string(expanded_control_bits[kRight]), debug);
    expanded_values[kLeft].PrintBlockHexTrace(LOCATION, current_level + "Expanded value (L): ", debug);
    expanded_values[kRight].PrintBlockHexTrace(LOCATION, current_level + "Expanded value (R): ", debug);
#endif

    // Apply correction word if control bit is set.
    if (current_control_bit) {
        expanded_seeds[kLeft]  = expanded_seeds[kLeft] ^ correction_word.seed;
        expanded_seeds[kRight] = expanded_seeds[kRight] ^ correction_word.seed;
        expanded_control_bits[kLeft] ^= correction_word.control_left;
        expanded_control_bits[kRight] ^= correction_word.control_right;
    }
}

}    // namespace dcf
}    // namespace fss
