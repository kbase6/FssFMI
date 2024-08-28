/**
 * @file fss_fmi_bench.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-27
 * @copyright Copyright (c) 2024
 * @brief FssFMI benchmark implementation.
 */

#include "fss_fmi.hpp"

#include <sdsl/csa_wt.hpp>
#include <sdsl/suffix_arrays.hpp>

#include "../../tools/random_number_generator.hpp"
#include "../../utils/file_io.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/timer.hpp"
#include "../../utils/utils.hpp"
#include "../internal/fsskey_io.hpp"

namespace {

const std::string kCurrentPath     = utils::GetCurrentDirectory();
const std::string kBenchFMIPath    = kCurrentPath + "/data/bench/fmi/";
const std::string kFMIBTPath_F     = kBenchFMIPath + "btf";
const std::string kFMIBTPath_F_P0  = kBenchFMIPath + "btf_p0";
const std::string kFMIBTPath_F_P1  = kBenchFMIPath + "btf_p1";
const std::string kFMIBTPath_G     = kBenchFMIPath + "btg";
const std::string kFMIBTPath_G_P0  = kBenchFMIPath + "btg_p0";
const std::string kFMIBTPath_G_P1  = kBenchFMIPath + "btg_p1";
const std::string kFMIKeyPath_P0   = kBenchFMIPath + "key_p0";
const std::string kFMIKeyPath_P1   = kBenchFMIPath + "key_p1";
const std::string kFMIDBPath       = kBenchFMIPath + "db";
const std::string kFMIBWTPath      = kBenchFMIPath + "bwt";
const std::string kFMIQueryPath    = kBenchFMIPath + "query";
const std::string kFMIQueryPath_P0 = kBenchFMIPath + "query_p0";
const std::string kFMIQueryPath_P1 = kBenchFMIPath + "query_p1";

using bts_t = tools::secret_sharing::bts_t;

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

void GenerateRandomNumbers(std::vector<uint32_t> &vec, const uint32_t bitsize) {
    // Generate random vector
    for (size_t i = 0; i < vec.size(); i++) {
        vec[i] = utils::Mod(tools::rng::SecureRng::Rand64(), bitsize);
    }
}

}    // namespace

namespace fss {
namespace fmi {
namespace bench {

void Bench_FssFmi(tools::secret_sharing::Party &party, const BenchInfo &bench_info) {
    // Define utilities
    utils::ExecutionTimer               timer_all, timer_1, timer_2;
    utils::FileIo                       io;
    tools::secret_sharing::ShareHandler sh;
    internal::FssKeyIo                  key_io;

    std::vector<std::string> modes         = {"Measurement of share generation", "Measurement of FssFMI key", "Measurement of execute Eval^{FssFMI}"};
    uint32_t                 selected_mode = bench_info.mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    for (const auto t : bench_info.text_size) {
        for (const auto q : bench_info.query_size) {
            for (uint32_t i = 0; i < bench_info.experiment_num; i++) {
                FssFmiParameters                             params(t, q, bench_info.dbg_info);
                uint32_t                                     ts = params.text_size;
                uint32_t                                     qs = params.query_size;
                tools::secret_sharing::AdditiveSecretSharing ss(t);
                FssFmi                                       fss_fmi(params);
                utils::Logger::InfoLog(LOCATION, "FssFMI: (text size, query size) = (" + std::to_string(t) + ", " + std::to_string(q) + ")");

                // Measure total time
                std::string mode_str     = "[" + modes[selected_mode - 1] + "],";
                std::string measure_info = "Info,Text size,Query size,Time";
                utils::Logger::InfoLog(LOCATION, mode_str + measure_info);
                measure_info            = "," + std::to_string(t) + "," + std::to_string(q);
                std::string file_option = "_t" + std::to_string(t) + "_q" + std::to_string(q);
                timer_all.Start();
                // ############# START #############

                if (selected_mode == 1) {
                    // Generate data
                    timer_1.Start();
                    std::vector<uint32_t> pub_db(ts - 1);
                    std::vector<uint32_t> q(qs);
                    GenerateRandomNumbers(pub_db, 1);
                    GenerateRandomNumbers(q, 1);
                    std::reverse(pub_db.begin(), pub_db.end());    // To find LPM, we need to reverse the text
                    std::string bwt = ConstructBwtFromVector(utils::VectorToStr(pub_db, ""));
                    io.WriteVectorToFile(kFMIDBPath + "_t" + std::to_string(t), pub_db);
                    io.WriteVectorToFile(kFMIQueryPath + file_option, q);
                    io.WriteStringToFile(kFMIBWTPath + "_t" + std::to_string(t), bwt);
                    timer_1.Print(LOCATION, mode_str + "Generate data" + measure_info);

                    // Generate query shares
                    timer_1.Start();
                    std::pair<std::vector<uint32_t>, std::vector<uint32_t>> q_sh = ss.Share(q);
                    sh.ExportShare(kFMIQueryPath_P0 + file_option, kFMIQueryPath_P1 + file_option, q_sh);
                    timer_1.Print(LOCATION, mode_str + "Generate share of query" + measure_info);

                } else if (selected_mode == 2) {
                    timer_all.SetTimeUnit(utils::TimeUnit::MICROSECONDS);
                    timer_1.SetTimeUnit(utils::TimeUnit::MICROSECONDS);

                    // Generate shares of beaver triples
                    timer_1.Start();
                    bts_t btf(qs), btg(qs);
                    ss.GenerateBeaverTriples(qs, btf);
                    ss.GenerateBeaverTriples(qs, btg);
                    std::pair<bts_t, bts_t> btf_sh = ss.ShareBeaverTriples(btf);
                    std::pair<bts_t, bts_t> btg_sh = ss.ShareBeaverTriples(btg);
                    sh.ExportBT(kFMIBTPath_F + file_option, btf);
                    sh.ExportBT(kFMIBTPath_G + file_option, btg);
                    sh.ExportBTShare(kFMIBTPath_F_P0 + file_option, kFMIBTPath_F_P1 + file_option, btf_sh);
                    sh.ExportBTShare(kFMIBTPath_G_P0 + file_option, kFMIBTPath_G_P1 + file_option, btg_sh);
                    timer_1.Print(LOCATION, mode_str + "Generate share of beaver triples" + measure_info);

                    // Generate key of FssFMI
                    timer_1.Start();
                    std::pair<FssFmiKey, FssFmiKey> fmi_keys = fss_fmi.GenerateKeys(qs - 1, qs);
                    key_io.WriteFssFmiKeyToFile(kFMIKeyPath_P0 + file_option, fmi_keys.first);
                    key_io.WriteFssFmiKeyToFile(kFMIKeyPath_P1 + file_option, fmi_keys.second);
                    timer_1.Print(LOCATION, mode_str + "Generate FssFMI key" + measure_info);

                } else if (selected_mode == 3) {
                    // Start communication
                    party.StartCommunication();

                    timer_1.Start();
                    // Set database (bwt)
                    std::string bwt;
                    io.ReadStringFromFile(kFMIBWTPath + "_t" + std::to_string(t), bwt);
                    fss_fmi.SetSentence(bwt);
                    // Set beaver triples
                    bts_t btf, btg;
                    if (party.GetId() == 0) {
                        sh.LoadBTShare(kFMIBTPath_F_P0 + file_option, btf);
                        sh.LoadBTShare(kFMIBTPath_G_P0 + file_option, btg);
                    } else {
                        sh.LoadBTShare(kFMIBTPath_F_P1 + file_option, btf);
                        sh.LoadBTShare(kFMIBTPath_G_P1 + file_option, btg);
                    }
                    fss_fmi.SetBeaverTriple(btf, btg);
                    // Read FssFMI key
                    FssFmiKey fmi_key;
                    if (party.GetId() == 0) {
                        key_io.ReadFssFmiKeyFromFile(kFMIKeyPath_P0 + file_option, params, fmi_key);
                    } else {
                        key_io.ReadFssFmiKeyFromFile(kFMIKeyPath_P1 + file_option, params, fmi_key);
                    }
                    // Read input data
                    std::vector<uint32_t> q_0(qs), q_1(qs);
                    if (party.GetId() == 0) {
                        sh.LoadShare(kFMIQueryPath_P0 + file_option, q_0);
                    } else {
                        sh.LoadShare(kFMIQueryPath_P1 + file_option, q_1);
                    }
                    timer_1.Print(LOCATION, mode_str + "Set data" + measure_info);

                    // Execute Eval^{FssFMI} algorithm
                    timer_2.Start();
                    std::vector<uint32_t> eq(qs), eq_0(qs), eq_1(qs);
                    if (party.GetId() == 0) {
                        fss_fmi.Evaluate(party, fmi_key, q_0, eq_0);
                    } else {
                        fss_fmi.Evaluate(party, fmi_key, q_1, eq_1);
                    }
                    ss.Reconst(party, eq_0, eq_1, eq);
                    timer_2.Print(LOCATION, mode_str + "Execute Eval^{FssFMI}" + measure_info);
                    fmi_key.FreeFssFmiKey();
                    timer_1.Print(LOCATION, mode_str + "FssFMI Total time" + measure_info);
                    party.OutputTotalBytesSent(measure_info);
                }

                // ############# END #############
                double timer_res = timer_all.Print(LOCATION, mode_str + "Bench Total time" + measure_info);
                if (utils::ExecutionTimer::IsExceedLimitTime(timer_res, bench_info.limit_time_ms, timer_all.GetTimeUnit())) {
                    utils::Logger::InfoLog(LOCATION, "The execution time exceeds the limit time: " + std::to_string(timer_res) + " " + timer_all.GetTimeUnitStr());
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

}    // namespace bench
}    // namespace fmi
}    // namespace fss
