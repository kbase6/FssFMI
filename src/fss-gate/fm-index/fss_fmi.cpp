/**
 * @file fss_fmi.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-25
 * @copyright Copyright (c) 2024
 * @brief FssFmi implementation.
 */

#include "fss_fmi.hpp"

#include <algorithm>

#include "../../tools/random_number_generator.hpp"
#include "../../tools/secret_sharing.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/timer.hpp"
#include "../../utils/utils.hpp"

namespace fss {
namespace fmi {

FssFmiParameters::FssFmiParameters()
    : text_bitsize(0), text_size(0), query_bitsize(0), query_size(0), debug(false) {
}

FssFmiParameters::FssFmiParameters(const uint32_t t, const uint32_t q, const DebugInfo &dbg_info)
    : text_bitsize(t), text_size(utils::Pow(2, t)), query_bitsize(q), query_size(utils::Pow(2, q)), rank_params(rank::FssRankParameters(t, dbg_info)), zt_params(zt::ZeroTestParameters(t, t, dbg_info)), debug(dbg_info.fmi_debug), dbg_info(dbg_info) {
    // : text_bitsize(t), text_size(utils::Pow(2, t)), query_bitsize(q), query_size(utils::Pow(2, q)), rank_params(rank::FssRankParameters(t, dbg_info)), zt_params(zt::ZeroTestParameters(t, 1, dbg_info)), debug(dbg_info.fmi_debug), dbg_info(dbg_info) {
}

FssFmiKey::FssFmiKey(const uint32_t rank_key_num, const uint32_t zt_key_num)
    : rank_key_num(rank_key_num), zt_key_num(zt_key_num) {
}

void FssFmiKey::PrintFssFmiKey(const FssFmiParameters &params, const bool debug) const {
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("FssFMI key"), debug);
    for (uint32_t i = 0; i < rank_key_num; i++) {
        this->rank_keys_f[i].PrintFssRankKey(params.rank_params, debug);
        this->rank_keys_g[i].PrintFssRankKey(params.rank_params, debug);
    }
    for (uint32_t i = 0; i < zt_key_num; i++) {
        this->zt_keys[i].PrintZeroTestKey(params.zt_params, debug);
    }
    utils::Logger::TraceLog(LOCATION, utils::kDash, debug);
#endif
}

void FssFmiKey::FreeFssFmiKey() {
    for (uint32_t i = 0; i < rank_key_num; i++) {
        this->rank_keys_f[i].FreeFssRankKey();
        this->rank_keys_g[i].FreeFssRankKey();
    }
    for (uint32_t i = 0; i < zt_key_num; i++) {
        this->zt_keys[i].FreeZeroTestKey();
    }
}

FssFmi::FssFmi(const FssFmiParameters params)
    : params_(params), rank_(rank::FssRank(params.rank_params)), zt_(params.zt_params) {
}

void FssFmi::SetBeaverTriple(const tools::secret_sharing::bts_t &btf, const tools::secret_sharing::bts_t &btg) {
    this->btf_ = std::move(btf);
    this->btg_ = std::move(btg);
}

void FssFmi::SetSentence(const std::string &sentence) {
    this->pub_db_ = sentence;
    this->cf1_    = std::count(sentence.begin(), sentence.end(), '0');
#ifdef LOG_LEVEL_TRACE
    utils::Logger::TraceLog(LOCATION, "cf1: " + std::to_string(this->cf1_), this->params_.debug);
#endif
}

std::pair<FssFmiKey, FssFmiKey> FssFmi::GenerateKeys(const uint32_t rank_key_num, const uint32_t zt_key_num) const {
#ifdef LOG_LEVEL_TRACE
    bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Generate FssFMI keys"), debug);
    utils::Logger::TraceLog(LOCATION, "Zero Test: (text size, query size) = (" + std::to_string(this->params_.text_bitsize) + ", " + std::to_string(this->params_.query_bitsize) + ")", debug);
#endif

    std::array<FssFmiKey, 2> fmi_key{FssFmiKey(rank_key_num, zt_key_num), FssFmiKey(rank_key_num, zt_key_num)};

    std::pair<std::vector<rank::FssRankKey>, std::vector<rank::FssRankKey>> rank_keys_f, rank_keys_g;
    rank_keys_f.first.reserve(rank_key_num);
    rank_keys_f.second.reserve(rank_key_num);
    rank_keys_g.first.reserve(rank_key_num);
    rank_keys_g.second.reserve(rank_key_num);
    for (uint32_t i = 0; i < rank_key_num; i++) {
        std::pair<rank::FssRankKey, rank::FssRankKey> rank_key_f = this->rank_.GenerateKeys();
        std::pair<rank::FssRankKey, rank::FssRankKey> rank_key_g = this->rank_.GenerateKeys();
        rank_keys_f.first.push_back(std::move(rank_key_f.first));
        rank_keys_f.second.push_back(std::move(rank_key_f.second));
        rank_keys_g.first.push_back(std::move(rank_key_g.first));
        rank_keys_g.second.push_back(std::move(rank_key_g.second));
    }

    std::pair<std::vector<zt::ZeroTestKey>, std::vector<zt::ZeroTestKey>> zt_keys;
    zt_keys.first.reserve(zt_key_num);
    zt_keys.second.reserve(zt_key_num);
    for (uint32_t i = 0; i < zt_key_num; i++) {
        std::pair<zt::ZeroTestKey, zt::ZeroTestKey> zt_key = this->zt_.GenerateKeys();
        zt_keys.first.push_back(std::move(zt_key.first));
        zt_keys.second.push_back(std::move(zt_key.second));
    }

    fmi_key[0].rank_keys_f = std::move(rank_keys_f.first);
    fmi_key[1].rank_keys_f = std::move(rank_keys_f.second);
    fmi_key[0].rank_keys_g = std::move(rank_keys_g.first);
    fmi_key[1].rank_keys_g = std::move(rank_keys_g.second);
    fmi_key[0].zt_keys     = std::move(zt_keys.first);
    fmi_key[1].zt_keys     = std::move(zt_keys.second);

#ifdef LOG_LEVEL_TRACE
    utils::AddNewLine(debug);
    fmi_key[0].PrintFssFmiKey(this->params_, debug);
    utils::AddNewLine(debug);
    fmi_key[1].PrintFssFmiKey(this->params_, debug);
    utils::AddNewLine(debug);
#endif

    return std::make_pair(std::move(fmi_key[0]), std::move(fmi_key[1]));
}

void FssFmi::Evaluate(tools::secret_sharing::Party &party, const FssFmiKey &fmi_key, const std::vector<uint32_t> &q, std::vector<uint32_t> &output) const {
    uint32_t                                     t  = this->params_.text_bitsize;
    uint32_t                                     ts = this->params_.text_size;
    uint32_t                                     qs = this->params_.query_size;
    tools::secret_sharing::AdditiveSecretSharing ss(t);
    utils::ExecutionTimer                        timer;

#ifdef LOG_LEVEL_TRACE
    const bool debug = this->params_.debug;
    utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate FssFmi"), debug);
    utils::Logger::TraceLog(LOCATION, "q: " + utils::VectorToStr(q), debug);
    utils::Logger::TraceLog(LOCATION, "(text size, query size): (" + std::to_string(ts) + ", " + std::to_string(qs) + ")", debug);
#endif

    uint32_t              fsh_0{0}, fsh_1{0}, gsh_0{0}, gsh_1{0};
    std::vector<uint32_t> intersh_0(qs), intersh_1(qs);

    // Calculate f_1, g_1
    if (party.GetId() == 0) {
        fsh_0        = utils::Mod(this->cf1_ * q[0], t);
        gsh_0        = utils::Mod((ts - 1 - this->cf1_) * q[0], t);
        intersh_0[0] = utils::Mod(gsh_0 - fsh_0, t);
    } else {
        fsh_1        = utils::Mod(this->cf1_ * q[0] + 1, t);
        gsh_1        = utils::Mod(this->cf1_ + ((ts - 1 - this->cf1_) * q[0]) + 1, t);
        intersh_1[0] = utils::Mod(gsh_1 - fsh_1, t);
    }

#ifdef LOG_LEVEL_TRACE
    // Debug: Reconst
    uint32_t f{0}, g{0};
    f = ss.Reconst(party, fsh_0, fsh_1);
    g = ss.Reconst(party, gsh_0, gsh_1);
    utils::Logger::TraceLog(LOCATION, "f_0: 0, g_0: " + std::to_string(ts), debug);
    utils::Logger::TraceLog(LOCATION, "f_1: " + std::to_string(f) + ", g_1: " + std::to_string(g), debug);
#endif

    // Update f_i, g_i
    for (uint32_t i = 1; i < qs; i++) {
        // Reconst f - r_in, g - r_in
        // timer.Start();
        std::array<uint32_t, 2> fgr_0{0, 0}, fgr_1{0, 0}, fgr{0, 0};
        if (party.GetId() == 0) {
            fgr_0[0] = utils::Mod(fsh_0 - fmi_key.rank_keys_f[i - 1].shr_in, t);
            fgr_0[1] = utils::Mod(gsh_0 - fmi_key.rank_keys_g[i - 1].shr_in, t);
        } else {
            fgr_1[0] = utils::Mod(fsh_1 - fmi_key.rank_keys_f[i - 1].shr_in, t);
            fgr_1[1] = utils::Mod(gsh_1 - fmi_key.rank_keys_g[i - 1].shr_in, t);
        }
        ss.Reconst(party, fgr_0, fgr_1, fgr);    // * ROUND: 1

        // Calculate rank f, g
        std::array<uint32_t, 2> rankf_0{0, 0}, rankf_1{0, 0}, rankg_0{0, 0}, rankg_1{0, 0};
        if (party.GetId() == 0) {
            rankf_0 = this->rank_.Evaluate(fmi_key.rank_keys_f[i - 1], this->pub_db_, fgr[0]);
            rankg_0 = this->rank_.Evaluate(fmi_key.rank_keys_g[i - 1], this->pub_db_, fgr[1]);
        } else {
            rankf_1 = this->rank_.Evaluate(fmi_key.rank_keys_f[i - 1], this->pub_db_, fgr[0]);
            rankg_1 = this->rank_.Evaluate(fmi_key.rank_keys_g[i - 1], this->pub_db_, fgr[1]);
        }
#ifdef LOG_LEVEL_TRACE
        // Debug: Reconst rank
        utils::Logger::TraceLog(LOCATION, "fr_" + std::to_string(i + 1) + ": " + std::to_string(fgr[0]) + ", gr_" + std::to_string(i + 1) + ": " + std::to_string(fgr[1]), debug);
        std::vector<uint32_t> rankf(2), rankg(2);
        std::vector<uint32_t> rf_0{rankf_0[0], rankf_0[1]}, rf_1{rankf_1[0], rankf_1[1]}, rg_0{rankg_0[0], rankg_0[1]}, rg_1{rankg_1[0], rankg_1[1]};
        ss.Reconst(party, rf_0, rf_1, rankf);
        ss.Reconst(party, rg_0, rg_1, rankg);
        utils::Logger::TraceLog(LOCATION, "rankf0_" + std::to_string(i + 1) + ": " + std::to_string(rankf[0]) + ", rankf1_" + std::to_string(i + 1) + ": " + std::to_string(rankf[1]), debug);
        utils::Logger::TraceLog(LOCATION, "rankg0_" + std::to_string(i + 1) + ": " + std::to_string(rankg[0]) + ", rankg1_" + std::to_string(i + 1) + ": " + std::to_string(rankg[1]), debug);
#endif

        // rank_0 if q[i] = 0 else rank_1
        std::array<uint32_t, 2> mfg_0 = {0, 0}, mfg_1 = {0, 0};
        if (party.GetId() == 0) {
            mfg_0 = ss.Mult2(party, this->btf_[i - 1], this->btg_[i - 1], q[i], utils::Mod(rankf_0[1] - rankf_0[0], t), q[i], utils::Mod(rankg_0[1] - rankg_0[0], t));
            fsh_0 = utils::Mod(rankf_0[0] + mfg_0[0], t);
            gsh_0 = utils::Mod(rankg_0[0] + mfg_0[1], t);
        } else {
            mfg_1 = ss.Mult2(party, this->btf_[i - 1], this->btg_[i - 1], q[i], utils::Mod(rankf_1[1] - rankf_1[0], t), q[i], utils::Mod(rankg_1[1] - rankg_1[0], t));
            fsh_1 = utils::Mod(rankf_1[0] + mfg_1[0], t);
            gsh_1 = utils::Mod(rankg_1[0] + mfg_1[1], t);
        }

        // Add CF_1
        if (party.GetId() == 0) {
            fsh_0        = utils::Mod(fsh_0 + (this->cf1_ * q[i]), t);
            gsh_0        = utils::Mod(gsh_0 + (this->cf1_ * q[i]), t);
            intersh_0[i] = utils::Mod(gsh_0 - fsh_0, t);
        } else {
            fsh_1        = utils::Mod(fsh_1 + (this->cf1_ * q[i]) + 1, t);
            gsh_1        = utils::Mod(gsh_1 + (this->cf1_ * q[i]) + 1, t);
            intersh_1[i] = utils::Mod(gsh_1 - fsh_1, t);
        }
#ifdef LOG_LEVEL_TRACE
        // Debug: Reconst f, g
        f = ss.Reconst(party, fsh_0, fsh_1);
        g = ss.Reconst(party, gsh_0, gsh_1);
        utils::Logger::TraceLog(LOCATION, "f_" + std::to_string(i + 1) + ": " + std::to_string(f) + ", g_" + std::to_string(i + 1) + ": " + std::to_string(g), debug);
        if (f > ts || g > ts) {
            utils::Logger::FatalLog(LOCATION, "f: " + std::to_string(f) + ", g: " + std::to_string(g) + " is out of range");
            exit(EXIT_FAILURE);
        }
#endif
        // timer.Print(LOCATION, "Evaluate FssFmi" + std::to_string(i + 1));
    }

    // Equality check of f, g
    std::vector<uint32_t> xsh_0(qs), xsh_1(qs), xr(qs);
    for (uint32_t i = 0; i < qs; i++) {
        if (party.GetId() == 0) {
            xsh_0[i] = utils::Mod(intersh_0[i] + fmi_key.zt_keys[i].shr_in, t);
        } else {
            xsh_1[i] = utils::Mod(intersh_1[i] + fmi_key.zt_keys[i].shr_in, t);
        }
    }
    ss.Reconst(party, xsh_0, xsh_1, xr);    // * ROUND: 3
    for (uint32_t i = 0; i < qs; i++) {
        output[i] = this->zt_.EvaluateAt(fmi_key.zt_keys[i], xr[i]);
    }
}

}    // namespace fmi
}    // namespace fss
