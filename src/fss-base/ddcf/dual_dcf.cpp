/**
 * @file dual_dcf.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-02-05
 * @copyright Copyright (c) 2024
 * @brief Dual DCF (DDCF) implementation.
 */

#include "dual_dcf.hpp"

#include "../../tools/random_number_generator.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"

namespace fss {
namespace ddcf {

DdcfParameters::DdcfParameters()
    : input_bitsize(0), element_bitsize(0), debug(false), dbg_info(DebugInfo()) {
}

DdcfParameters::DdcfParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info)
    : input_bitsize(n), element_bitsize(e),
      debug(dbg_info.ddcf_debug), dbg_info(dbg_info) {
}

void DdcfKey::PrintDdcfKey(const bool debug) const {
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("DDCF Key"), debug);
    this->dcf_key.PrintDcfKey(debug);
    utils::Logger::TraceLog(LOCATION, "Mask: " + std::to_string(this->mask), debug);
    utils::Logger::TraceLog(LOCATION, utils::kDash, debug);
#endif
}

void DdcfKey::FreeDdcfKey() {
    this->dcf_key.FreeDcfKey();
}

DualDistributedComparisonFunction::DualDistributedComparisonFunction(const DdcfParameters params)
    : params_(params),
      dcf_(dcf::DistributedComparisonFunction(
          dcf::DcfParameters(params.input_bitsize, params.element_bitsize, params.dbg_info))) {
}

std::pair<DdcfKey, DdcfKey> DualDistributedComparisonFunction::GenerateKeys(const uint32_t alpha, const uint32_t beta_1, const uint32_t beta_2) const {
    uint32_t e = this->params_.element_bitsize;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Generate DDCF keys"), debug);
    utils::Logger::TraceLog(LOCATION, "Alpha: " + std::to_string(alpha), debug);
#endif

    std::array<DdcfKey, 2> keys;

    // line 1: calculate beta
    uint32_t beta = utils::Mod(beta_1 - beta_2, e);
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "beta: " + std::to_string(beta) + " (" + std::to_string(beta_1) + " - " + std::to_string(beta_2) + ")", debug);
#endif
    // line 2: generate DCF key
    std::pair<dcf::DcfKey, dcf::DcfKey> dcf_keys = this->dcf_.GenerateKeys(alpha, beta);
    // line 3: generate share of beta_2
    keys[0].mask = utils::Mod(tools::rng::SecureRng().Rand64(), e);
    keys[1].mask = utils::Mod(beta_2 - keys[0].mask, e);
    // line 4: set DCF key
    keys[0].dcf_key = std::move(dcf_keys.first);
    keys[1].dcf_key = std::move(dcf_keys.second);

#ifdef LOG_LEVEL_TRACE
    utils::AddNewLine(debug);
    keys[0].PrintDdcfKey(debug);
    utils::AddNewLine(debug);
    keys[1].PrintDdcfKey(debug);
    utils::AddNewLine(debug);
#endif

    return std::make_pair(std::move(keys[0]), std::move(keys[1]));
}

uint32_t DualDistributedComparisonFunction::EvaluateAt(const DdcfKey &ddcf_key, uint32_t x) const {
    uint32_t e = this->params_.element_bitsize;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate input with DDCF key"), debug);
    utils::Logger::TraceLog(LOCATION, "Input: " + std::to_string(x), debug);
#endif

    // line 2: evaluate key
    uint32_t output = this->dcf_.EvaluateAt(std::move(ddcf_key.dcf_key), x);
    // line 3: mask
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(utils::Mod(output + ddcf_key.mask, e)) + " (" + std::to_string(output) + "+" + std::to_string(ddcf_key.mask) + ")", debug);
#endif
    output = utils::Mod(output + ddcf_key.mask, e);
    return output;
}

}    // namespace ddcf
}    // namespace fss
