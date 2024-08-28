/**
 * @file distributed_comparison_function_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-02-05
 * @copyright Copyright (c) 2024
 * @brief DCF test implementation.
 */

#include "distributed_comparison_function.hpp"

#include "../../tools/random_number_generator.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"

namespace fss {
namespace dcf {
namespace test {

bool Test_EvaluateSinglePoint(const TestInfo &test_info);

void Test_Dcf(TestInfo &test_info) {
    std::vector<std::string> modes = {
        "DCF unit tests",
        "EvaluateSinglePoint",
    };
    uint32_t selected_mode = test_info.mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    utils::PrintText(utils::Logger::StrWithSep(modes[selected_mode - 1]));
    if (selected_mode == 1) {
        test_info.dbg_info.debug = false;
        utils::PrintTestResult("Test_EvaluateSinglePoint", Test_EvaluateSinglePoint(test_info));
    } else if (selected_mode == 2) {
        utils::PrintTestResult("Test_EvaluateSinglePoint", Test_EvaluateSinglePoint(test_info));
    }
    utils::PrintText(utils::kDash);
}

bool Test_EvaluateSinglePoint(const TestInfo &test_info) {
    bool                    result = true;
    uint32_t                x, res;
    std::array<uint32_t, 2> sh_res;
    for (const auto size : test_info.domain_size) {
        // Set DCF parameters
        DcfParameters                 params(size, size, test_info.dbg_info);
        int                           e = params.element_bitsize;
        DistributedComparisonFunction dcf(params);

        // Set input values
        uint32_t alpha = 0b00011;    // 3
        uint32_t beta  = 0b00010;    // 2

        // Generate DCF keys
        std::pair<DcfKey, DcfKey> dcf_keys = dcf.GenerateKeys(alpha, beta);

        // Evaluate DCF
        x         = 0b00010;    // 2
        sh_res[0] = dcf.EvaluateAt(std::move(dcf_keys.first), x);
        sh_res[1] = dcf.EvaluateAt(std::move(dcf_keys.second), x);
        res       = utils::Mod(sh_res[0] + sh_res[1], e);
        result &= (res == beta);
        if (!result) {
            utils::Logger::DebugLog(LOCATION, "x=" + std::to_string(x) + " -> Result: " + std::to_string(res) + " (x_0, x_1) = (" + std::to_string(sh_res[0]) + ", " + std::to_string(sh_res[1]) + ")", test_info.dbg_info.debug);
        }

        x         = 0b00111;    // 7
        sh_res[0] = dcf.EvaluateAt(std::move(dcf_keys.first), x);
        sh_res[1] = dcf.EvaluateAt(std::move(dcf_keys.second), x);
        res       = utils::Mod(sh_res[0] + sh_res[1], e);
        result &= (res == 0);
        if (!result) {
            utils::Logger::DebugLog(LOCATION, "x=" + std::to_string(x) + " -> Result: " + std::to_string(res) + " (x_0, x_1) = (" + std::to_string(sh_res[0]) + ", " + std::to_string(sh_res[1]) + ")", test_info.dbg_info.debug);
        }

        dcf_keys.first.FreeDcfKey();
        dcf_keys.second.FreeDcfKey();
    }
    return result;
}

}    // namespace test
}    // namespace dcf
}    // namespace fss
