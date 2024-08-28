/**
 * @file fss_rank.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-24
 * @copyright Copyright (c) 2024
 * @brief FssRank implementation.
 */

#include "fss_rank.hpp"

#include "../../tools/random_number_generator.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/timer.hpp"
#include "../../utils/utils.hpp"

#include <algorithm>

namespace {

void CalculateReverseCumulativeSum(std::vector<uint32_t> &vec, const uint32_t bitsize) {
    uint32_t tmp = vec[vec.size() - 1];    // Assign the last value
    for (long i = vec.size() - 2; i >= 0; --i) {
        tmp    = utils::Mod(tmp + vec[i], bitsize);    // Calculate the cumulative sum
        vec[i] = tmp;
    }
}

void RotateRight(std::vector<uint32_t> &vec, size_t n) {
    std::rotate(vec.rbegin(), vec.rbegin() + n, vec.rend());
}

}    // namespace

namespace fss {
namespace rank {

FssRankParameters::FssRankParameters()
    : text_bitsize(0), debug(false) {
}

FssRankParameters::FssRankParameters(const uint32_t t, const DebugInfo &dbg_info)
    : text_bitsize(t), dpf_params(dpf::DpfParameters(t, t, dbg_info)), debug(dbg_info.rank_debug), dbg_info(dbg_info) {
}

FssRankKey::FssRankKey()
    : shr_in(0) {
}

void FssRankKey::PrintFssRankKey(const FssRankParameters &params, const bool debug) const {
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("FssRank Key"), debug);
    this->dpf_key.PrintDpfKey(params.dpf_params, debug);
    utils::Logger::TraceLog(LOCATION, "Share(r_in): " + std::to_string(this->shr_in), debug);
    utils::Logger::TraceLog(LOCATION, utils::kDash, debug);
#endif
}

void FssRankKey::FreeFssRankKey() {
    this->dpf_key.FreeDpfKey();
}

FssRank::FssRank(const FssRankParameters params)
    : params_(params),
      dpf_(params.dpf_params) {
}

std::pair<FssRankKey, FssRankKey> FssRank::GenerateKeys() const {
    uint32_t t = this->params_.text_bitsize;
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Generate FssRank keys"), debug);
    utils::Logger::TraceLog(LOCATION, "Rank: (text size) = (" + std::to_string(t) + ")", debug);
#endif

    std::array<FssRankKey, 2> rank_key;

    // Generate DPF keys
    uint32_t                            r_in = utils::Mod(tools::rng::SecureRng().Rand64(), t);
    std::pair<dpf::DpfKey, dpf::DpfKey> keys = this->dpf_.GenerateKeys(r_in, 1);

    // Generate share of r_in
    rank_key[0].shr_in = utils::Mod(tools::rng::SecureRng().Rand64(), t);
    rank_key[1].shr_in = utils::Mod(r_in - rank_key[0].shr_in, t);
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "r_in: " + std::to_string(r_in) + " -> (" + std::to_string(rank_key[0].shr_in) + ", " + std::to_string(rank_key[1].shr_in) + ")", debug);
#endif

    // Set DPF keys
    rank_key[0].dpf_key = std::move(keys.first);
    rank_key[1].dpf_key = std::move(keys.second);

#ifdef LOG_LEVEL_TRACE
    utils::AddNewLine(debug);
    rank_key[0].PrintFssRankKey(this->params_, debug);
    utils::AddNewLine(debug);
    rank_key[1].PrintFssRankKey(this->params_, debug);
    utils::AddNewLine(debug);
#endif

    return std::make_pair(std::move(rank_key[0]), std::move(rank_key[1]));
}

std::array<uint32_t, 2> FssRank::Evaluate(const FssRankKey &rank_key, const std::string &sentence, const uint32_t pos) const {
    uint32_t t = this->params_.text_bitsize;

#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Calculate rank value"), debug);
#endif

    // Setup DPF key and evaluate full domain
    std::vector<uint32_t> outputs(utils::Pow(2, t));
    this->dpf_.EvaluateFullDomain(rank_key.dpf_key, outputs);

    // Rotate the output vector
    RotateRight(outputs, pos - 1);
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "RotateRight: " + utils::VectorToStr(outputs), debug);
#endif

    // Calculate the reverse cumulative sum
    CalculateReverseCumulativeSum(outputs, t);
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "ReverseCumulativeSum: " + utils::VectorToStr(outputs), debug);
#endif

    // Calculate the rank value
    std::array<uint32_t, 2> rank = {0, 0};
    for (size_t i = 0; i < sentence.size(); i++) {
        if (sentence[i] == '0') {
            rank[0] = utils::Mod(rank[0] + outputs[i], t);
        } else if (sentence[i] == '1') {
            rank[1] = utils::Mod(rank[1] + outputs[i], t);
        } else {
        }
    }

#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "Rank: (" + std::to_string(rank[0]) + ", " + std::to_string(rank[1]) + ")", debug);
#endif

    return rank;
}

}    // namespace rank
}    // namespace fss
