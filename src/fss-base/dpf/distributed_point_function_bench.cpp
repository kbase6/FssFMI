/**
 * @file distributed_point_function_bench.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-29
 * @copyright Copyright (c) 2024
 * @brief DPF benchmark implementation.
 */

#include "distributed_point_function.hpp"

#include "../../tools/random_number_generator.hpp"
#include "../../tools/secret_sharing.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/timer.hpp"
#include "../../utils/utils.hpp"

namespace fss {
namespace dpf {
namespace bench {

void Bench_Dpf(const BenchInfo &bench_info) {
    // Define utilities
    utils::ExecutionTimer timer_all, timer_1, timer_2;

    std::vector<std::string> modes         = {"Evaluate Full Domain", "Evaluate Full Domain (1-bit)", "Evaluate Full Domain Non Recursive", "Evaluate Full Domain Recursive", "Evaluate Full Domain Naive"};
    int                      selected_mode = bench_info.mode;
    if (selected_mode < 1 || selected_mode > static_cast<int>(modes.size())) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    for (const auto t : bench_info.text_size) {
        for (uint32_t i = 0; i < bench_info.experiment_num; i++) {
            DpfParameters            params(t, t, bench_info.dbg_info);
            DpfParameters            params2(t, 1, bench_info.dbg_info);
            uint32_t                 fde_size = utils::Pow(2, t);
            DistributedPointFunction dpf(params);
            DistributedPointFunction dpf_one(params2);

            // Measure total time
            std::string mode_str     = "[" + modes[selected_mode - 1] + "],";
            std::string measure_info = "Info,Text size,Time";
            utils::Logger::InfoLog(LOCATION, mode_str + measure_info);
            measure_info            = "," + std::to_string(t);
            std::string file_option = "_t" + std::to_string(t);
            timer_all.Start();
            // ############# START #############

            uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), t);
            uint32_t beta  = utils::Mod(tools::rng::SecureRng().Rand32(), t);

            if (selected_mode == 1) {
                utils::Logger::InfoLog(LOCATION, "DPF: (input size, element size, terminate size) = (" + std::to_string(params.input_bitsize) + ", " + std::to_string(params.element_bitsize) + ", " + std::to_string(params.terminate_bitsize) + ")");
                timer_1.SetTimeUnit(utils::TimeUnit::NANOSECONDS);
                timer_1.Start();
                std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeys(alpha, beta);
                timer_1.Print(LOCATION, mode_str + "Gen Key" + measure_info);
                timer_1.SetTimeUnit(utils::TimeUnit::MICROSECONDS);

                timer_1.Start();
                std::vector<uint32_t> res_fde(fde_size);
                dpf.EvaluateFullDomain(std::move(dpf_keys.first), res_fde);
                timer_1.Print(LOCATION, mode_str + "Eval Full Domain Opt" + measure_info);
                dpf_keys.first.FreeDpfKey();
                dpf_keys.second.FreeDpfKey();
            } else if (selected_mode == 2) {
                utils::Logger::InfoLog(LOCATION, "DPF: (input size, element size, terminate size) = (" + std::to_string(params2.input_bitsize) + ", " + std::to_string(params2.element_bitsize) + ", " + std::to_string(params2.terminate_bitsize) + ")");
                timer_1.SetTimeUnit(utils::TimeUnit::NANOSECONDS);
                timer_1.Start();
                std::pair<DpfKey, DpfKey> dpf_keys = dpf_one.GenerateKeys(alpha, beta);
                timer_1.Print(LOCATION, mode_str + "Gen Key" + measure_info);
                timer_1.SetTimeUnit(utils::TimeUnit::MICROSECONDS);

                timer_1.Start();
                std::vector<uint32_t> res_fde(fde_size);
                dpf_one.EvaluateFullDomainOneBit(std::move(dpf_keys.first), res_fde);
                timer_1.Print(LOCATION, mode_str + "Eval Full Domain 1bit" + measure_info);
                dpf_keys.first.FreeDpfKey();
                dpf_keys.second.FreeDpfKey();
            } else if (selected_mode == 3) {
                utils::Logger::InfoLog(LOCATION, "DPF: (input size, element size, terminate size) = (" + std::to_string(params.input_bitsize) + ", " + std::to_string(params.element_bitsize) + ", " + std::to_string(params.terminate_bitsize) + ")");
                timer_1.SetTimeUnit(utils::TimeUnit::NANOSECONDS);
                timer_1.Start();
                std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeys(alpha, beta);
                timer_1.Print(LOCATION, mode_str + "Gen Key" + measure_info);
                timer_1.SetTimeUnit(utils::TimeUnit::MICROSECONDS);

                timer_1.Start();
                std::vector<uint32_t> res_fde(fde_size);
                dpf.FullDomainNonRecursive(std::move(dpf_keys.first), res_fde);
                timer_1.Print(LOCATION, mode_str + "Eval Non Recursive" + measure_info);
                dpf_keys.first.FreeDpfKey();
                dpf_keys.second.FreeDpfKey();
            } else if (selected_mode == 4) {
                utils::Logger::InfoLog(LOCATION, "DPF: (input size, element size, terminate size) = (" + std::to_string(params.input_bitsize) + ", " + std::to_string(params.element_bitsize) + ", " + std::to_string(params.terminate_bitsize) + ")");
                timer_1.Start();
                timer_1.SetTimeUnit(utils::TimeUnit::NANOSECONDS);
                std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeysNaive(alpha, beta);
                timer_1.Print(LOCATION, mode_str + "Gen Key" + measure_info);
                timer_1.SetTimeUnit(utils::TimeUnit::MICROSECONDS);

                timer_1.Start();
                std::vector<uint32_t> res_naive(fde_size);
                dpf.FullDomainRecursive(std::move(dpf_keys.first), res_naive);
                timer_1.Print(LOCATION, mode_str + "Eval Recursive" + measure_info);
                dpf_keys.first.FreeDpfKey();
                dpf_keys.second.FreeDpfKey();
            } else if (selected_mode == 5) {
                utils::Logger::InfoLog(LOCATION, "DPF: (input size, element size, terminate size) = (" + std::to_string(params.input_bitsize) + ", " + std::to_string(params.element_bitsize) + ", " + std::to_string(params.terminate_bitsize) + ")");
                timer_1.Start();
                timer_1.SetTimeUnit(utils::TimeUnit::NANOSECONDS);
                std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeysNaive(alpha, beta);
                timer_1.Print(LOCATION, mode_str + "Gen Key" + measure_info);
                timer_1.SetTimeUnit(utils::TimeUnit::MICROSECONDS);

                timer_1.Start();
                std::vector<uint32_t> res_naive(fde_size);
                dpf.FullDomainNaiveNaive(std::move(dpf_keys.first), res_naive);
                timer_1.Print(LOCATION, mode_str + "Eval Naive" + measure_info);
                dpf_keys.first.FreeDpfKey();
                dpf_keys.second.FreeDpfKey();
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

}    // namespace bench
}    // namespace dpf
}    // namespace fss
