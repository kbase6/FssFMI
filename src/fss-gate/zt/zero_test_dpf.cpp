/**
 * @file zero_test_dpf.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-19
 * @copyright Copyright (c) 2024
 * @brief Zero Test implementation.
 */

#include "zero_test_dpf.hpp"

#include "../../tools/random_number_generator.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"

namespace fss {
namespace zt {

ZeroTestParameters::ZeroTestParameters()
    : input_bitsize(0), element_bitsize(0), debug(false) {
}

ZeroTestParameters::ZeroTestParameters(const uint32_t n, const uint32_t e, const DebugInfo &dbg_info)
    : input_bitsize(n), element_bitsize(e), dpf_params(dpf::DpfParameters(n, e, dbg_info)), debug(dbg_info.debug), dbg_info(dbg_info) {
}

ZeroTestKey::ZeroTestKey()
    : shr_in(0) {
}

void ZeroTestKey::PrintZeroTestKey(const ZeroTestParameters &params, const bool debug) const {
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Zero Test Key"), debug);
    this->dpf_key.PrintDpfKey(params.dpf_params, debug, false);
    utils::Logger::TraceLog(LOCATION, "Share(r_in): " + std::to_string(this->shr_in), debug);
    utils::Logger::TraceLog(LOCATION, utils::kDash, debug);
#endif
}

void ZeroTestKey::FreeZeroTestKey() {
    this->dpf_key.FreeDpfKey();
}

ZeroTest::ZeroTest(const ZeroTestParameters params)
    : params_(params),
      dpf_(dpf::DistributedPointFunction(params.dpf_params)) {
}

std::pair<ZeroTestKey, ZeroTestKey> ZeroTest::GenerateKeys() const {
    uint32_t n = this->params_.input_bitsize;
#ifdef LOG_LEVEL_DEBUG
    uint32_t e     = this->params_.element_bitsize;
    bool     debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Generate Zero Test keys"), debug);
    utils::Logger::TraceLog(LOCATION, "(input size, element size) = (" + std::to_string(n) + ", " + std::to_string(e) + ")", debug);
#endif

    std::array<ZeroTestKey, 2> keys;
    // Sample random number
    uint32_t r_in = utils::Mod(tools::rng::SecureRng().Rand64(), n);
#ifdef LOG_LEVEL_DEBUG
    utils::Logger::TraceLog(LOCATION, "r_in: " + std::to_string(r_in), debug);
#endif
    std::pair<dpf::DpfKey, dpf::DpfKey> dpf_keys = this->dpf_.GenerateKeys(r_in, 1);

    keys[0].shr_in  = utils::Mod(tools::rng::SecureRng().Rand64(), n);
    keys[1].shr_in  = utils::Mod(r_in - keys[0].shr_in, n);
    keys[0].dpf_key = std::move(dpf_keys.first);
    keys[1].dpf_key = std::move(dpf_keys.second);

#ifdef LOG_LEVEL_TRACE
    utils::AddNewLine(debug);
    keys[0].PrintZeroTestKey(this->params_, debug);
    utils::AddNewLine(debug);
    keys[1].PrintZeroTestKey(this->params_, debug);
    utils::AddNewLine(debug);
#endif

    // Return the generated keys
    return std::make_pair(std::move(keys[0]), std::move(keys[1]));
}

uint32_t ZeroTest::EvaluateAt(const ZeroTestKey &zt_key, const uint32_t x) const {
    uint32_t output = this->dpf_.EvaluateAt(zt_key.dpf_key, x);
#ifdef LOG_LEVEL_DEBUG
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate input with Zero Test key"), debug);
    utils::Logger::TraceLog(LOCATION, "Output: " + std::to_string(output), debug);
#endif
    return output;
}

}    // namespace zt
}    // namespace fss
