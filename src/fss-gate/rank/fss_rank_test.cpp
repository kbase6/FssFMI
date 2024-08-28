/**
 * @file fss_rank_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-24
 * @copyright Copyright (c) 2024
 * @brief FssRank test implementation.
 */

#include "fss_rank.hpp"

#include <thread>

#include "../../tools/random_number_generator.hpp"
#include "../../utils/file_io.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"
#include "../internal/fsskey_io.hpp"

namespace {

const std::string kCurrentPath             = utils::GetCurrentDirectory();
const std::string kTestRankPath            = kCurrentPath + "/data/test/rank/";
const std::string kRankKeyPath_P0          = kTestRankPath + "key_p0";
const std::string kRankKeyPath_P1          = kTestRankPath + "key_p1";
const std::string kRankDBPath              = kTestRankPath + "db";
const std::string kRankPosPath             = kTestRankPath + "pos";
const std::string kRankPosSharePath_P0     = kTestRankPath + "pos_share_p0";
const std::string kRankPosSharePath_P1     = kTestRankPath + "pos_share_p1";
const std::string kRankAlpPath             = kTestRankPath + "alp";
const std::string kRankAlpSharePath_P0     = kTestRankPath + "alp_share_p0";
const std::string kRankAlpSharePath_P1     = kTestRankPath + "alp_share_p1";
const std::string kRankBeaverTriplePath    = kTestRankPath + "bt";
const std::string kRankBeaverTriplePath_P0 = kTestRankPath + "bt_p0";
const std::string kRankBeaverTriplePath_P1 = kTestRankPath + "bt_p1";

using bts_t = tools::secret_sharing::bts_t;

uint32_t Rank(const std::string &bit_array, const uint32_t index, const char alp) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < index; i++) {
        if (bit_array[i] == alp) {
            count++;
        }
    }
    return count;
}

std::string GenerateBinaryString(const uint32_t length) {
    std::string result;
    for (uint32_t i = 0; i < length; i++) {
        result += std::to_string(tools::rng::SecureRng::RandBool());
    }
    return result;
}

}    // namespace

namespace fss {
namespace rank {
namespace test {

bool Test_FssRankOffline(tools::secret_sharing::Party &party, const TestInfo &test_info);
bool Test_FssRankOnline(tools::secret_sharing::Party &party, const TestInfo &test_info);

void Test_FssRank(tools::secret_sharing::Party &party, TestInfo &test_info) {
    std::vector<std::string> modes         = {"FssRank unit tests", "FssRankOffline", "FssRankOnline"};
    uint32_t                 selected_mode = test_info.mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    utils::PrintText(utils::Logger::StrWithSep(modes[selected_mode - 1]));
    if (selected_mode == 1) {
        test_info.dbg_info.debug = false;
        if (party.GetId() == 0) {
            utils::PrintTestResult("Test_FssRankOffline", Test_FssRankOffline(party, test_info));
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        utils::PrintTestResult("Test_FssRankOnline", Test_FssRankOnline(party, test_info));
    } else if (selected_mode == 2) {
        utils::PrintTestResult("Test_FssRankOffline", Test_FssRankOffline(party, test_info));
    } else if (selected_mode == 3) {
        utils::PrintTestResult("Test_FssRankOnline", Test_FssRankOnline(party, test_info));
    }
    utils::PrintText(utils::kDash);
}

bool Test_FssRankOffline(tools::secret_sharing::Party &party, const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Set parameters
        FssRankParameters                            params(size, test_info.dbg_info);
        uint32_t                                     ts = utils::Pow(2, size);
        tools::secret_sharing::AdditiveSecretSharing ss(size);
        utils::FileIo                                io;
        tools::secret_sharing::ShareHandler          sh;
        internal::FssKeyIo                           key_io(test_info.dbg_info.debug);
        FssRank                                      fss_rank(params);

        // Generate share of data
        std::string db = GenerateBinaryString(ts);
        uint32_t    pos, alp;
        alp                                  = tools::rng::SecureRng::RandBool();
        pos                                  = utils::Mod(tools::rng::SecureRng::Rand32(), size);
        std::pair<uint32_t, uint32_t> pos_sh = ss.Share(pos);
        std::pair<uint32_t, uint32_t> alp_sh = ss.Share(alp);

        io.WriteStringToFile(kRankDBPath, db);
        io.WriteValueToFile(kRankAlpPath, alp);
        io.WriteValueToFile(kRankPosPath, pos);
        sh.ExportShare(kRankPosSharePath_P0, kRankPosSharePath_P1, pos_sh);
        sh.ExportShare(kRankAlpSharePath_P0, kRankAlpSharePath_P1, alp_sh);

        if (size < 10) {
            utils::Logger::DebugLog(LOCATION, "db   : " + db, test_info.dbg_info.debug);
        }
        utils::Logger::DebugLog(LOCATION, "pos  : " + std::to_string(pos), test_info.dbg_info.debug);
        utils::Logger::DebugLog(LOCATION, "pos_0: " + std::to_string(pos_sh.first), test_info.dbg_info.debug);
        utils::Logger::DebugLog(LOCATION, "pos_1: " + std::to_string(pos_sh.second), test_info.dbg_info.debug);
        utils::Logger::DebugLog(LOCATION, "alp  : " + std::to_string(alp), test_info.dbg_info.debug);
        utils::Logger::DebugLog(LOCATION, "alp_0: " + std::to_string(alp_sh.first), test_info.dbg_info.debug);
        utils::Logger::DebugLog(LOCATION, "alp_1: " + std::to_string(alp_sh.second), test_info.dbg_info.debug);

        // Generate beaver triples
        tools::secret_sharing::bts_t bt_vec(1);
        ss.GenerateBeaverTriples(1, bt_vec);
        std::pair<bts_t, bts_t> bt_vec_sh = ss.ShareBeaverTriples(bt_vec);
        sh.ExportBT(kRankBeaverTriplePath, bt_vec);
        sh.ExportBTShare(kRankBeaverTriplePath_P0, kRankBeaverTriplePath_P1, bt_vec_sh);
        for (size_t i = 0; i < bt_vec.size(); i++) {
            utils::Logger::DebugLog(LOCATION, "Share of bt: " + bt_vec[i].ToStr() + " -> " + bt_vec_sh.first[i].ToStr(false) + ", " + bt_vec_sh.second[i].ToStr(false), test_info.dbg_info.debug);
        }

        // Generate key of FssRank
        std::pair<rank::FssRankKey, rank::FssRankKey> rank_keys = fss_rank.GenerateKeys();
        key_io.WriteFssRankKeyToFile(kRankKeyPath_P0, rank_keys.first);
        key_io.WriteFssRankKeyToFile(kRankKeyPath_P1, rank_keys.second);
        FssRankKey rank_key_0, rank_key_1;
        key_io.ReadFssRankKeyFromFile(kRankKeyPath_P0, params, rank_key_0);
        key_io.ReadFssRankKeyFromFile(kRankKeyPath_P1, params, rank_key_1);
        result &= (rank_keys.first == rank_key_0) && (rank_keys.second == rank_key_1);

        rank_keys.first.FreeFssRankKey();
        rank_keys.second.FreeFssRankKey();
        rank_key_0.FreeFssRankKey();
        rank_key_1.FreeFssRankKey();
    }
    return result;
}

bool Test_FssRankOnline(tools::secret_sharing::Party &party, const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        FssRankParameters                            params(size, test_info.dbg_info);
        tools::secret_sharing::AdditiveSecretSharing ss(size);
        utils::FileIo                                io;
        tools::secret_sharing::ShareHandler          sh;
        internal::FssKeyIo                           key_io(test_info.dbg_info.debug);
        FssRank                                      fss_rank(params);

        // Set database
        utils::Logger::DebugLog(LOCATION, "Read database from file.", test_info.dbg_info.debug);
        std::string db;
        io.ReadStringFromFile(kRankDBPath, db);

        // Set beaver triple
        utils::Logger::DebugLog(LOCATION, "Read beaver triple.", test_info.dbg_info.debug);
        bts_t bt_vec_0, bt_vec_1;
        if (party.GetId() == 0) {
            sh.LoadBTShare(kRankBeaverTriplePath_P0, bt_vec_0);
        } else {
            sh.LoadBTShare(kRankBeaverTriplePath_P1, bt_vec_1);
        }

        // Read FssRank key
        utils::Logger::DebugLog(LOCATION, "Read rank key.", test_info.dbg_info.debug);
        FssRankKey rank_key;
        if (party.GetId() == 0) {
            key_io.ReadFssRankKeyFromFile(kRankKeyPath_P0, params, rank_key);
        } else {
            key_io.ReadFssRankKeyFromFile(kRankKeyPath_P1, params, rank_key);
        }

        // Read input data
        utils::Logger::DebugLog(LOCATION, "Read input data from file.", test_info.dbg_info.debug);
        uint32_t pos(0), pos_0(0), pos_1(0), alp(0), alp_0(0), alp_1(0);
        io.ReadValueFromFile(kRankPosPath, pos);
        io.ReadValueFromFile(kRankAlpPath, alp);
        if (party.GetId() == 0) {
            io.ReadValueFromFile(kRankPosSharePath_P0, pos_0);
            io.ReadValueFromFile(kRankAlpSharePath_P0, alp_0);
        } else {
            io.ReadValueFromFile(kRankPosSharePath_P1, pos_1);
            io.ReadValueFromFile(kRankAlpSharePath_P1, alp_1);
        }

        uint32_t posr(0), posr_0(0), posr_1(0);
        // Reconst pos - r_in
        if (party.GetId() == 0) {
            posr_0 = utils::Mod(pos_0 - rank_key.shr_in, size);
        } else {
            posr_1 = utils::Mod(pos_1 - rank_key.shr_in, size);
        }

        party.StartCommunication();

        posr = ss.Reconst(party, posr_0, posr_1);
        utils::Logger::DebugLog(LOCATION, "posr: " + std::to_string(posr), test_info.dbg_info.debug);

        // Evaluate rank
        utils::Logger::DebugLog(LOCATION, "Evaluate rank.", test_info.dbg_info.debug);
        std::array<uint32_t, 2> rank_p0{0, 0}, rank_p1{0, 0};
        if (party.GetId() == 0) {
            rank_p0 = fss_rank.Evaluate(rank_key, db, posr);
        } else {
            rank_p1 = fss_rank.Evaluate(rank_key, db, posr);
        }

        // Select alphabet (0 or 1)
        uint32_t z_0{0}, z_1{0}, res_0{0}, res_1{0};
        if (party.GetId() == 0) {
            z_0   = ss.Mult(party, bt_vec_0[0], alp_0, utils::Mod(rank_p0[1] - rank_p0[0], size));
            res_0 = utils::Mod(rank_p0[0] + z_0, size);
        } else {
            z_1   = ss.Mult(party, bt_vec_1[0], alp_1, utils::Mod(rank_p1[1] - rank_p1[0], size));
            res_1 = utils::Mod(rank_p1[0] + z_1, size);
        }
        uint32_t res = ss.Reconst(party, res_0, res_1);

        // Check
        uint32_t t_rank = Rank(db, pos, alp ? '1' : '0');
        utils::Logger::DebugLog(LOCATION, "Position: " + std::to_string(pos) + ", Alphabet: " + std::to_string(alp), test_info.dbg_info.debug);
        utils::Logger::DebugLog(LOCATION, "Correct rank: " + std::to_string(t_rank) + ", Evaluated rank: " + std::to_string(res), test_info.dbg_info.debug);
        result &= (res == t_rank);

        rank_key.FreeFssRankKey();
    }
    return result;
}

}    // namespace test
}    // namespace rank
}    // namespace fss
