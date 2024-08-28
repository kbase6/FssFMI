/**
 * @file dual_dcf_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-02-05
 * @copyright Copyright (c) 2024
 * @brief Dual DCF (DDCF) test implementation.
 */

#include "dual_dcf.hpp"

#include "../../tools/random_number_generator.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"

namespace fss {
namespace ddcf {
namespace test {

bool Test_EvaluateSinglePoint(const TestInfo &test_info);

void Test_Ddcf(TestInfo &test_info) {

    std::vector<std::string> modes         = {"DDCF unit tests", "EvaluateSinglePoint"};
    uint32_t                 selected_mode = test_info.mode;
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
        // Set DDCF parameters
        DdcfParameters                    params(size, size, test_info.dbg_info);
        int                               e = params.element_bitsize;
        DualDistributedComparisonFunction ddcf(params);

        // Set input values
        uint32_t alpha = 0b00011;    // 3
        uint32_t beta1 = 0b00010;    // 2
        uint32_t beta2 = 0b00100;    // 4

        // Generate DDCF keys
        std::pair<DdcfKey, DdcfKey> ddcf_keys = ddcf.GenerateKeys(alpha, beta1, beta2);

        // Evaluate DDCF
        x         = 0b00010;    // 2
        sh_res[0] = ddcf.EvaluateAt(std::move(ddcf_keys.first), x);
        sh_res[1] = ddcf.EvaluateAt(std::move(ddcf_keys.second), x);
        res       = utils::Mod(sh_res[0] + sh_res[1], e);
        result &= (res == beta1);
        if (!result) {
            utils::Logger::DebugLog(LOCATION, "x=" + std::to_string(x) + " -> Result: " + std::to_string(res) + " (x_0, x_1) = (" + std::to_string(sh_res[0]) + ", " + std::to_string(sh_res[1]) + ")", test_info.dbg_info.debug);
        }

        x         = 0b00111;    // 7
        sh_res[0] = ddcf.EvaluateAt(std::move(ddcf_keys.first), x);
        sh_res[1] = ddcf.EvaluateAt(std::move(ddcf_keys.second), x);
        res       = utils::Mod(sh_res[0] + sh_res[1], e);
        result &= (res == beta2);
        if (!result) {
            utils::Logger::DebugLog(LOCATION, "x=" + std::to_string(x) + " -> Result: " + std::to_string(res) + " (x_0, x_1) = (" + std::to_string(sh_res[0]) + ", " + std::to_string(sh_res[1]) + ")", test_info.dbg_info.debug);
        }

        ddcf_keys.first.FreeDdcfKey();
        ddcf_keys.second.FreeDdcfKey();
    }
    return result;
}

}    // namespace test
}    // namespace ddcf
}    // namespace fss
