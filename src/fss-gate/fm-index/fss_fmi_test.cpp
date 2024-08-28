/**
 * @file fss_fmi_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-25
 * @copyright Copyright (c) 2024
 * @brief FssFmi test implementation.
 */

#include "fss_fmi.hpp"

#include <sdsl/csa_wt.hpp>
#include <sdsl/suffix_arrays.hpp>
#include <thread>

#include "../../tools/random_number_generator.hpp"
#include "../../utils/file_io.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"
#include "../internal/fsskey_io.hpp"

namespace {

const std::string kCurrentPath     = utils::GetCurrentDirectory();
const std::string kTestFMIPath     = kCurrentPath + "/data/test/fmi/";
const std::string kFMIBTPath_F     = kTestFMIPath + "btf";
const std::string kFMIBTPath_F_P0  = kTestFMIPath + "btf_p0";
const std::string kFMIBTPath_F_P1  = kTestFMIPath + "btf_p1";
const std::string kFMIBTPath_G     = kTestFMIPath + "btg";
const std::string kFMIBTPath_G_P0  = kTestFMIPath + "btg_p0";
const std::string kFMIBTPath_G_P1  = kTestFMIPath + "btg_p1";
const std::string kFMIKeyPath_P0   = kTestFMIPath + "key_p0";
const std::string kFMIKeyPath_P1   = kTestFMIPath + "key_p1";
const std::string kFMIDBPath       = kTestFMIPath + "db";
const std::string kFMIBWTPath      = kTestFMIPath + "bwt";
const std::string kFMIQueryPath    = kTestFMIPath + "query";
const std::string kFMIQueryPath_P0 = kTestFMIPath + "query_p0";
const std::string kFMIQueryPath_P1 = kTestFMIPath + "query_p1";

using bts_t = tools::secret_sharing::bts_t;

constexpr uint32_t kQuerySize = 4;

std::string ConstructBwtFromVector(const std::string &input) {
    size_t input_size = input.size();
    // Construct the suffix array using the SDSL library
    sdsl::csa_wt<> csa;
    sdsl::construct_im(csa, input, 1);
    // Convert the BWT to a string
    std::string bwt_vector = "";
    for (size_t i = 0; i < input_size + 1; i++) {
        if (csa.bwt[i]) {
            bwt_vector += csa.bwt[i];
        } else {
            bwt_vector += '$';
        }
    }
    return bwt_vector;
}

void CalculateFmindex(const std::string text, std::string query, const uint32_t query_pos, const bool debug) {
    if (debug) {
        size_t post_context = 3;
        size_t pre_context  = 3;

        // FMインデックスを構築
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<127>>, 512, 1024> fm_index;
        sdsl::construct_im(fm_index, text, 1);    // インデックスの生成

        query       = query.substr(0, query_pos);
        size_t m    = query.size();
        size_t occs = sdsl::count(fm_index, query.begin(), query.end());
        std::cout << "# of occurrences: " << occs << std::endl;

        if (occs > 0) {
            std::cout << "Location and context of first occurrences: " << std::endl;
            auto locations = locate(fm_index, query.begin(), query.begin() + m);
            std::sort(locations.begin(), locations.end());
            for (size_t i = 0, pre_extract = pre_context, post_extract = post_context; i < occs; ++i) {
                std::cout << std::setw(8) << locations[i] << ": ";
                if (pre_extract > locations[i]) {
                    pre_extract = locations[i];
                }
                if (locations[i] + m + post_extract > fm_index.size()) {
                    post_extract = fm_index.size() - locations[i] - m;
                }
                auto        s   = extract(fm_index, locations[i] - pre_extract, locations[i] + m + post_extract - 1);
                std::string pre = s.substr(0, pre_extract);
                s               = s.substr(pre_extract);
                if (pre.find_last_of('\n') != std::string::npos) {
                    pre = pre.substr(pre.find_last_of('\n') + 1);
                }
                std::cout << pre;
                std::cout << "\e[1;31m";
                std::cout << s.substr(0, m);
                std::cout << "\e[0m";
                std::string context = s.substr(m);
                std::cout << context.substr(0, context.find_first_of('\n')) << std::endl;
            }
        }
    }
}

uint32_t FindIndicesOfOnes(const std::vector<uint32_t> &input_vector) {
    uint32_t one_index = input_vector.size();
    for (size_t i = 0; i < input_vector.size(); ++i) {
        if (input_vector[i] == 1) {
            one_index = i;
            break;
        }
    }
    return one_index;
}

void GenerateRandomNumbers(std::vector<uint32_t> &vec, const uint32_t bitsize) {
    // Generate random vector
    for (size_t i = 0; i < vec.size(); i++) {
        vec[i] = utils::Mod(tools::rng::SecureRng::Rand64(), bitsize);
    }
}

}    // namespace

namespace fss {
namespace fmi {
namespace test {

bool Test_FssFMIOffline(tools::secret_sharing::Party &party, const TestInfo &test_info);
bool Test_FssFMIOnline(tools::secret_sharing::Party &party, const TestInfo &test_info);

void Test_FssFmi(tools::secret_sharing::Party &party, TestInfo &test_info) {
    std::vector<std::string> modes         = {"FssFMI unit tests", "FssFMIOffline", "FssFMIOnline"};
    uint32_t                 selected_mode = test_info.mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    utils::PrintText(utils::Logger::StrWithSep(modes[selected_mode - 1]));
    if (selected_mode == 1) {
        test_info.dbg_info.debug = false;
        if (party.GetId() == 0) {
            utils::PrintTestResult("Test_FssFMIOffline", Test_FssFMIOffline(party, test_info));
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        utils::PrintTestResult("Test_FssFMIOnline", Test_FssFMIOnline(party, test_info));
    } else if (selected_mode == 2) {
        utils::PrintTestResult("Test_FssFMIOffline", Test_FssFMIOffline(party, test_info));
    } else if (selected_mode == 3) {
        utils::PrintTestResult("Test_FssFMIOnline", Test_FssFMIOnline(party, test_info));
    }
    utils::PrintText(utils::kDash);
}

bool Test_FssFMIOffline(tools::secret_sharing::Party &party, const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        FssFmiParameters                             params(size, kQuerySize, test_info.dbg_info);
        uint32_t                                     ts = params.text_size;
        uint32_t                                     qs = params.query_size;
        tools::secret_sharing::AdditiveSecretSharing ss(size);
        utils::FileIo                                io;
        tools::secret_sharing::ShareHandler          sh;
        internal::FssKeyIo                           key_io(test_info.dbg_info.debug);
        FssFmi                                       fss_fmi(params);

        std::vector<uint32_t> pub_db(ts - 1);
        std::vector<uint32_t> q(qs);

        GenerateRandomNumbers(pub_db, 1);
        GenerateRandomNumbers(q, 1);

        io.WriteVectorToFile(kFMIDBPath, pub_db);
        io.WriteVectorToFile(kFMIQueryPath, q);
        // To find LPM, we need to reverse the text
        std::reverse(pub_db.begin(), pub_db.end());
        std::string bwt = ConstructBwtFromVector(utils::VectorToStr(pub_db, ""));
        io.WriteStringToFile(kFMIBWTPath, bwt);

        std::pair<std::vector<uint32_t>, std::vector<uint32_t>> q_sh = ss.Share(q);
        sh.ExportShare(kFMIQueryPath_P0, kFMIQueryPath_P1, q_sh);

        utils::Logger::DebugLog(LOCATION, "Generate share of data.", test_info.dbg_info.debug);
        if (size < 10) {
            utils::Logger::DebugLog(LOCATION, "db : " + utils::VectorToStr(pub_db), test_info.dbg_info.debug);
            utils::Logger::DebugLog(LOCATION, "bwt: " + bwt, test_info.dbg_info.debug);
        }
        utils::Logger::DebugLog(LOCATION, "q  : " + utils::VectorToStr(q), test_info.dbg_info.debug);
        utils::Logger::DebugLog(LOCATION, "q_0: " + utils::VectorToStr(q_sh.first), test_info.dbg_info.debug);
        utils::Logger::DebugLog(LOCATION, "q_1: " + utils::VectorToStr(q_sh.second), test_info.dbg_info.debug);

        // Generate beaver triples
        bts_t btf(qs - 1), btg(qs - 1);
        ss.GenerateBeaverTriples(qs - 1, btf);
        ss.GenerateBeaverTriples(qs - 1, btg);
        std::pair<bts_t, bts_t> btf_sh = ss.ShareBeaverTriples(btf);
        std::pair<bts_t, bts_t> btg_sh = ss.ShareBeaverTriples(btg);
        sh.ExportBT(kFMIBTPath_F, btf);
        sh.ExportBT(kFMIBTPath_G, btg);
        sh.ExportBTShare(kFMIBTPath_F_P0, kFMIBTPath_F_P1, btf_sh);
        sh.ExportBTShare(kFMIBTPath_G_P0, kFMIBTPath_G_P1, btg_sh);
        for (uint32_t i = 0; i < qs - 1; i++) {
            utils::Logger::DebugLog(LOCATION, "Share of bt: " + btf[i].ToStr() + " -> " + btf_sh.first[i].ToStr(false) + ", " + btf_sh.second[i].ToStr(false), test_info.dbg_info.debug);
            utils::Logger::DebugLog(LOCATION, "Share of bt: " + btg[i].ToStr() + " -> " + btg_sh.first[i].ToStr(false) + ", " + btg_sh.second[i].ToStr(false), test_info.dbg_info.debug);
        }

        // Generate key of FssFMI
        std::pair<FssFmiKey, FssFmiKey> fmi_keys = fss_fmi.GenerateKeys(qs - 1, qs);
        utils::Logger::DebugLog(LOCATION, "Write FssFMI key to file.", test_info.dbg_info.debug);
        key_io.WriteFssFmiKeyToFile(kFMIKeyPath_P0, fmi_keys.first);
        key_io.WriteFssFmiKeyToFile(kFMIKeyPath_P1, fmi_keys.second);
        FssFmiKey fmi_key_0, fmi_key_1;
        key_io.ReadFssFmiKeyFromFile(kFMIKeyPath_P0, params, fmi_key_0);
        key_io.ReadFssFmiKeyFromFile(kFMIKeyPath_P1, params, fmi_key_1);
        result &= (fmi_keys.first == fmi_key_0) && (fmi_keys.second == fmi_key_1);

        fmi_keys.first.FreeFssFmiKey();
        fmi_keys.second.FreeFssFmiKey();
        fmi_key_0.FreeFssFmiKey();
        fmi_key_1.FreeFssFmiKey();
    }
    return result;
}

bool Test_FssFMIOnline(tools::secret_sharing::Party &party, const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        FssFmiParameters                             params(size, kQuerySize, test_info.dbg_info);
        uint32_t                                     qs = params.query_size;
        tools::secret_sharing::AdditiveSecretSharing ss(size);
        utils::FileIo                                io;
        tools::secret_sharing::ShareHandler          sh;
        internal::FssKeyIo                           key_io(test_info.dbg_info.debug);
        FssFmi                                       fss_fmi(params);

        // Set database (bwt)
        std::string bwt;
        io.ReadStringFromFile(kFMIBWTPath, bwt);
        fss_fmi.SetSentence(bwt);

        // Set beaver triples
        bts_t btf, btg;
        if (party.GetId() == 0) {
            sh.LoadBTShare(kFMIBTPath_F_P0, btf);
            sh.LoadBTShare(kFMIBTPath_G_P0, btg);
        } else {
            sh.LoadBTShare(kFMIBTPath_F_P1, btf);
            sh.LoadBTShare(kFMIBTPath_G_P1, btg);
        }
        fss_fmi.SetBeaverTriple(btf, btg);

        // Read FssFMI key
        FssFmiKey fmi_key;
        if (party.GetId() == 0) {
            key_io.ReadFssFmiKeyFromFile(kFMIKeyPath_P0, params, fmi_key);
        } else {
            key_io.ReadFssFmiKeyFromFile(kFMIKeyPath_P1, params, fmi_key);
        }

        // Read input data
        std::vector<uint32_t> q_0(qs), q_1(qs);
        if (party.GetId() == 0) {
            sh.LoadShare(kFMIQueryPath_P0, q_0);
        } else {
            sh.LoadShare(kFMIQueryPath_P1, q_1);
        }

        // Start communication
        party.StartCommunication();

        // Execute Eval^{FssFMI} algorithm
        std::vector<uint32_t> eq(qs), eq_0(qs), eq_1(qs);
        if (party.GetId() == 0) {
            fss_fmi.Evaluate(party, fmi_key, q_0, eq_0);
        } else {
            fss_fmi.Evaluate(party, fmi_key, q_1, eq_1);
        }
        ss.Reconst(party, eq_0, eq_1, eq);
        fmi_key.FreeFssFmiKey();

        utils::Logger::DebugLog(LOCATION, "Eval^{FssFMI} algorithm", test_info.dbg_info.debug);
        utils::Logger::DebugLog(LOCATION, "Eq: " + utils::VectorToStr(eq), test_info.dbg_info.debug);
        uint32_t one_index = FindIndicesOfOnes(eq);

        // Check the result
        std::vector<uint32_t> pub_db, q;
        io.ReadVectorFromFile(kFMIDBPath, pub_db);
        io.ReadVectorFromFile(kFMIQueryPath, q);
        std::string q_str = utils::VectorToStr(q, "");
        std::string text  = utils::VectorToStr(pub_db, "");
        if (size < 10) {
            utils::Logger::DebugLog(LOCATION, "Text  : " + text, test_info.dbg_info.debug);
            utils::Logger::DebugLog(LOCATION, "BWT   : " + bwt, test_info.dbg_info.debug);
        }
        utils::Logger::DebugLog(LOCATION, "Query : " + q_str, test_info.dbg_info.debug);
        q_str = q_str.substr(0, one_index);
        utils::Logger::DebugLog(LOCATION, "Match : " + q_str, test_info.dbg_info.debug);
        CalculateFmindex(text, q_str, q_str.size(), test_info.dbg_info.debug);
    }
    // TODO: Check the result
    return result;
}

}    // namespace test
}    // namespace fmi
}    // namespace fss
