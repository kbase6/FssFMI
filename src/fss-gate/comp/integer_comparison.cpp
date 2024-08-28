/**
 * @file integer_comparison.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-02-05
 * @copyright Copyright (c) 2024
 * @brief Integer Comparison implementation.
 */

#include "integer_comparison.hpp"

#include <bitset>

#include "../../tools/random_number_generator.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"

namespace {
constexpr int kDebugInputSize = 8;
}

namespace fss {
namespace comp {

CompParameters::CompParameters()
    : input_bitsize(0), element_bitsize(0), debug(false), dbg_info(DebugInfo()) {
}

CompParameters::CompParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info)
    : input_bitsize(n), element_bitsize(e),
      debug(dbg_info.comp_debug), dbg_info(dbg_info) {
}

CompKey::CompKey()
    : shr1_in(0), shr2_in(0), shr_out(0) {
}

void CompKey::PrintCompKey(const bool debug) const {
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Comp Key"), debug);
    this->ddcf_key.PrintDdcfKey(debug);
    utils::Logger::TraceLog(LOCATION, "Share(r1_in): " + std::to_string(this->shr1_in), debug);
    utils::Logger::TraceLog(LOCATION, "Share(r2_in): " + std::to_string(this->shr2_in), debug);
    utils::Logger::TraceLog(LOCATION, "Share(r_out): " + std::to_string(this->shr_out), debug);
    utils::Logger::TraceLog(LOCATION, utils::kDash, debug);
#endif
}

void CompKey::FreeCompKey() {
    this->ddcf_key.FreeDdcfKey();
}

IntegerComparison::IntegerComparison(CompParameters params)
    : params_(params),
      ddcf_(ddcf::DualDistributedComparisonFunction(
          ddcf::DdcfParameters(params.input_bitsize - 1, params.element_bitsize, params.dbg_info))) {
}

std::pair<CompKey, CompKey> IntegerComparison::GenerateKeys() const {
    uint32_t n = this->params_.input_bitsize;
    uint32_t e = this->params_.element_bitsize;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Generate COMP keys"), debug);
#endif

    std::array<CompKey, 2> keys;

    uint64_t r1_in = utils::Mod(tools::rng::SecureRng().Rand64(), n);
    uint64_t r2_in = utils::Mod(tools::rng::SecureRng().Rand64(), n);
    uint64_t r_out = utils::Mod(tools::rng::SecureRng().Rand64(), e);

    // line 1: calculate random number
    uint32_t r     = utils::Mod(utils::Pow(2, n) - (r1_in - r2_in), e);
    uint32_t alpha = utils::ExcludeBitsAbove(r, n);
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "r1_in: " + std::to_string(r1_in) + ", r2_in" + std::to_string(r2_in) + ", r: " + std::to_string(r), debug);
    utils::Logger::TraceLog(LOCATION, "alpha: " + std::to_string(alpha) + " (r: " + std::bitset<kDebugInputSize>(r).to_string() + ")", debug);
#endif

    // line 2: generate DDCF key
    bool msb_r = utils::GetBitAtPosition(r, n);
    // *不等号を反転させる
    uint32_t beta_1 = msb_r;
    uint32_t beta_2 = !msb_r;
    // uint32_t beta_1 = !msb_r;
    // uint32_t beta_2 = msb_r;
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "beta_1: " + std::to_string(beta_1) + ", beta_2: " + std::to_string(beta_2), debug);
#endif
    std::pair<ddcf::DdcfKey, ddcf::DdcfKey> ddcf_keys = this->ddcf_.GenerateKeys(alpha, beta_1, beta_2);
    // line 3: generate share of r_in, r_out
    keys[0].shr1_in = utils::Mod(tools::rng::SecureRng().Rand64(), n);
    keys[0].shr2_in = utils::Mod(tools::rng::SecureRng().Rand64(), n);
    keys[0].shr_out = utils::Mod(tools::rng::SecureRng().Rand64(), e);
    keys[1].shr1_in = utils::Mod(r1_in - keys[0].shr1_in, n);
    keys[1].shr2_in = utils::Mod(r2_in - keys[0].shr2_in, n);
    keys[1].shr_out = utils::Mod(r_out - keys[0].shr_out, e);
    // line 4: set DDCF key
    keys[0].ddcf_key = std::move(ddcf_keys.first);
    keys[1].ddcf_key = std::move(ddcf_keys.second);

#ifdef LOG_LEVEL_TRACE
    utils::AddNewLine(debug);
    keys[0].PrintCompKey(debug);
    utils::AddNewLine(debug);
    keys[1].PrintCompKey(debug);
    utils::AddNewLine(debug);
#endif

    return std::make_pair(std::move(keys[0]), std::move(keys[1]));
}

uint32_t IntegerComparison::Evaluate(const CompKey &comp_key, const uint32_t x, const uint32_t y) const {
    int n        = this->params_.input_bitsize;
    int e        = this->params_.element_bitsize;
    int party_id = comp_key.ddcf_key.dcf_key.party_id;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate input with COMP key"), debug);
#endif

    // line 2: set evaluate input
    uint32_t z     = utils::Mod(x - y, n);
    bool     msb_z = utils::GetBitAtPosition(z, n);
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "z: " + std::to_string(z) + " (=" + std::to_string(x) + "-" + std::to_string(y) + ")", debug);
    utils::Logger::TraceLog(LOCATION, "z[n-1]: " + std::to_string(msb_z) + " (" + std::bitset<kDebugInputSize>(z).to_string() + ")", debug);
#endif
    // line 3: evaluate key
    uint32_t zn     = utils::Mod(utils::Pow(2, n - 1) - utils::ExcludeBitsAbove(z, n) - 1, n - 1);
    uint32_t output = this->ddcf_.EvaluateAt(comp_key.ddcf_key, zn);
    output          = utils::Mod(party_id - ((party_id * msb_z) + output - (2 * msb_z * output)) + comp_key.shr_out, e);
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(output), debug);
#endif
    return output;
}

}    // namespace comp
}    // namespace fss
