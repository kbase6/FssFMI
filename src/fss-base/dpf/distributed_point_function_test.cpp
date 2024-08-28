/**
 * @file distributed_point_function_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-10
 * @copyright Copyright (c) 2024
 * @brief DPF test implementation.
 */

#include "distributed_point_function.hpp"

#include "../../tools/random_number_generator.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"

namespace {

bool DpfFullDomainCheck(const uint32_t alpha, const uint32_t beta, const std::vector<uint32_t> &res, const bool debug) {
    bool check = true;
    for (uint32_t i = 0; i < res.size(); i++) {
        if ((i == alpha && res[i] == beta) || (i != alpha && res[i] == 0)) {
            check &= true;
        } else {
            check &= false;
            utils::Logger::DebugLog(LOCATION, "FDE check failed at x=" + std::to_string(i) + " -> Result: " + std::to_string(res[i]), debug);
        }
    }
    return check;
}

}    // namespace

namespace fss {
namespace dpf {
namespace test {

bool Test_EvaluateSinglePoint(const TestInfo &test_info);
bool Test_EvaluateFullDomain(const TestInfo &test_info);
bool Test_EvaluateFullDomainOneBit(const TestInfo &test_info);
bool Test_FullDomainNonRecursiveParallel_4(const TestInfo &test_info);
bool Test_FullDomainNonRecursiveParallel_8(const TestInfo &test_info);
bool Test_FullDomainNonRecursive(const TestInfo &test_info);
bool Test_FullDomainRecursive(const TestInfo &test_info);
bool Test_FullDomainNaive(const TestInfo &test_info);

void Test_Dpf(TestInfo &test_info) {
    std::vector<std::string> modes         = {"DPF unit tests", "EvaluateSinglePoint", "EvaluateFullDomain", "EvaluateFullDomainOneBit", "FullDomainNonRecursiveParallel_4", "FullDomainNonRecursiveParallel_8", "FullDomainNonRecursive", "FullDomainRecursive", "FullDomainNaive"};
    uint32_t                 selected_mode = test_info.mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    utils::PrintText(utils::Logger::StrWithSep(modes[selected_mode - 1]));
    if (selected_mode == 1) {
        test_info.dbg_info.debug = false;
        utils::PrintTestResult("Test_EvaluateSinglePoint", Test_EvaluateSinglePoint(test_info));
        utils::PrintTestResult("Test_EvaluateFullDomain", Test_EvaluateFullDomain(test_info));
        utils::PrintTestResult("Test_EvaluateFullDomainOneBit", Test_EvaluateFullDomainOneBit(test_info));
        utils::PrintTestResult("Test_FullDomainNonRecursiveParallel_4(n=17~24)", Test_FullDomainNonRecursiveParallel_4(test_info));
        utils::PrintTestResult("Test_FullDomainNonRecursiveParallel_8(n=9~16)", Test_FullDomainNonRecursiveParallel_8(test_info));
        utils::PrintTestResult("Test_FullDomainNonRecursive(n=2~8)", Test_FullDomainNonRecursive(test_info));
        utils::PrintTestResult("Test_FullDomainRecursive", Test_FullDomainRecursive(test_info));
        utils::PrintTestResult("Test_FullDomainNaive", Test_FullDomainNaive(test_info));
    } else if (selected_mode == 2) {
        utils::PrintTestResult("Test_EvaluateSinglePoint", Test_EvaluateSinglePoint(test_info));
    } else if (selected_mode == 3) {
        utils::PrintTestResult("Test_EvaluateFullDomain", Test_EvaluateFullDomain(test_info));
    } else if (selected_mode == 4) {
        utils::PrintTestResult("Test_EvaluateFullDomainOneBit", Test_EvaluateFullDomainOneBit(test_info));
    } else if (selected_mode == 5) {
        utils::PrintTestResult("Test_FullDomainNonRecursiveParallel_4(n=17~24)", Test_FullDomainNonRecursiveParallel_4(test_info));
    } else if (selected_mode == 6) {
        utils::PrintTestResult("Test_FullDomainNonRecursiveParallel_8(n=9~16)", Test_FullDomainNonRecursiveParallel_8(test_info));
    } else if (selected_mode == 7) {
        utils::PrintTestResult("Test_FullDomainNonRecursive(n=2~8)", Test_FullDomainNonRecursive(test_info));
    } else if (selected_mode == 8) {
        utils::PrintTestResult("Test_FullDomainRecursive", Test_FullDomainRecursive(test_info));
    } else if (selected_mode == 9) {
        utils::PrintTestResult("Test_FullDomainNaive", Test_FullDomainNaive(test_info));
    }
    utils::PrintText(utils::kDash);
}

bool Test_EvaluateSinglePoint(const TestInfo &test_info) {
    bool                    result = true;
    uint32_t                x, res;
    std::array<uint32_t, 2> sh_res;
    for (const auto size : test_info.domain_size) {
        // Set DPF parameters
        DpfParameters            params(size, size, test_info.dbg_info);
        uint32_t                 e = params.element_bitsize;
        DistributedPointFunction dpf(params);

        // Set input values
        uint32_t alpha = 0b00011;    // 3
        uint32_t beta  = 0b00010;    // 2

        // Generate keys
        std::pair<DpfKey, DpfKey> dpf_keys       = dpf.GenerateKeys(alpha, beta);
        std::pair<DpfKey, DpfKey> dpf_keys_naive = dpf.GenerateKeysNaive(alpha, beta);

        // Evaluate DPF
        utils::Logger::DebugLog(LOCATION, "Early Termination", test_info.dbg_info.debug);
        x         = 0b00011;    // 3
        sh_res[0] = dpf.EvaluateAt(std::move(dpf_keys.first), x);
        sh_res[1] = dpf.EvaluateAt(std::move(dpf_keys.second), x);
        res       = utils::Mod(sh_res[0] + sh_res[1], e);
        result &= res == beta;
        if (!result) {
            utils::Logger::DebugLog(LOCATION, "x=" + std::to_string(x) + " -> Result: " + std::to_string(res) + " (x_0, x_1) = (" + std::to_string(sh_res[0]) + ", " + std::to_string(sh_res[1]) + ")", test_info.dbg_info.debug);
        }

        x         = 0b00111;    // 7
        sh_res[0] = dpf.EvaluateAt(std::move(dpf_keys.first), x);
        sh_res[1] = dpf.EvaluateAt(std::move(dpf_keys.second), x);
        res       = utils::Mod(sh_res[0] + sh_res[1], e);
        result &= res == 0;
        if (!result) {
            utils::Logger::DebugLog(LOCATION, "x=" + std::to_string(x) + " -> Result: " + std::to_string(res) + " (x_0, x_1) = (" + std::to_string(sh_res[0]) + ", " + std::to_string(sh_res[1]) + ")", test_info.dbg_info.debug);
        }

        utils::Logger::DebugLog(LOCATION, "Naive", test_info.dbg_info.debug);
        x         = 0b00011;    // 3
        sh_res[0] = dpf.EvaluateAtNaive(std::move(dpf_keys_naive.first), x);
        sh_res[1] = dpf.EvaluateAtNaive(std::move(dpf_keys_naive.second), x);
        res       = utils::Mod(sh_res[0] + sh_res[1], e);
        result &= res == beta;
        if (!result) {
            utils::Logger::DebugLog(LOCATION, "x=" + std::to_string(x) + " -> Result: " + std::to_string(res) + " (x_0, x_1) = (" + std::to_string(sh_res[0]) + ", " + std::to_string(sh_res[1]) + ")", test_info.dbg_info.debug);
        }

        x         = 0b00111;    // 7
        sh_res[0] = dpf.EvaluateAtNaive(std::move(dpf_keys_naive.first), x);
        sh_res[1] = dpf.EvaluateAtNaive(std::move(dpf_keys_naive.second), x);
        res       = utils::Mod(sh_res[0] + sh_res[1], e);
        result &= res == 0;
        if (!result) {
            utils::Logger::DebugLog(LOCATION, "x=" + std::to_string(x) + " -> Result: " + std::to_string(res) + " (x_0, x_1) = (" + std::to_string(sh_res[0]) + ", " + std::to_string(sh_res[1]) + ")", test_info.dbg_info.debug);
        }

        dpf_keys.first.FreeDpfKey();
        dpf_keys.second.FreeDpfKey();
        dpf_keys_naive.first.FreeDpfKey();
        dpf_keys_naive.second.FreeDpfKey();
    }
    return result;
}

bool Test_EvaluateFullDomain(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Set DPF parameters
        DpfParameters            params(size, size, test_info.dbg_info);
        uint32_t                 n        = params.input_bitsize;
        uint32_t                 e        = params.element_bitsize;
        uint32_t                 fde_size = utils::Pow(2, n);
        DistributedPointFunction dpf(params);

        // Set input values
        uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta  = utils::Mod(tools::rng::SecureRng().Rand32(), e);

        // Generate keys
        std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeys(alpha, beta);

        // Evaluate Full Domain of DPF
        std::vector<uint32_t> sh_0(fde_size), sh_1(fde_size), out(fde_size);

        dpf.EvaluateFullDomain(std::move(dpf_keys.first), sh_0);
        dpf.EvaluateFullDomain(std::move(dpf_keys.second), sh_1);
        for (uint32_t i = 0; i < fde_size; i++) {
            out[i] = utils::Mod(sh_0[i] + sh_1[i], e);
        }
        result &= DpfFullDomainCheck(alpha, beta, out, test_info.dbg_info.debug);

        dpf_keys.first.FreeDpfKey();
        dpf_keys.second.FreeDpfKey();
    }
    return result;
}

bool Test_EvaluateFullDomainOneBit(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : utils::CreateSequence(13, 28)) {
        // Set DPF parameters
        DpfParameters            params(size, 1, test_info.dbg_info);    // Input size equals to element size
        uint32_t                 n        = params.input_bitsize;
        uint32_t                 fde_size = utils::Pow(2, n);
        DistributedPointFunction dpf(params);

        // Set input values
        uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta  = 1;

        // Generate keys
        std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeys(alpha, beta);

        // Evaluate Full Domain of DPF
        std::vector<uint32_t> sh_0(fde_size), sh_1(fde_size), res(fde_size);

        dpf.EvaluateFullDomainOneBit(std::move(dpf_keys.first), sh_0);
        dpf.EvaluateFullDomainOneBit(std::move(dpf_keys.second), sh_1);
        for (uint32_t i = 0; i < fde_size; i++) {
            res[i] = sh_0[i] ^ sh_1[i];
        }
        result = DpfFullDomainCheck(alpha, beta, res, test_info.dbg_info.debug);

        dpf_keys.first.FreeDpfKey();
        dpf_keys.second.FreeDpfKey();
    }
    return result;
}

bool Test_FullDomainNonRecursiveParallel_4(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : utils::CreateSequence(17, 25)) {
        // Set DPF parameters
        DpfParameters            params(size, size, test_info.dbg_info);    // Input size equals to element size
        uint32_t                 n        = params.input_bitsize;
        uint32_t                 e        = params.element_bitsize;
        uint32_t                 fde_size = utils::Pow(2, n);
        DistributedPointFunction dpf(params);

        // Set input values
        uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta  = utils::Mod(tools::rng::SecureRng().Rand32(), e);

        // Generate keys
        std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeys(alpha, beta);

        // Evaluate Full Domain of DPF
        std::vector<uint32_t> sh_0(fde_size), sh_1(fde_size), res(fde_size);

        dpf.FullDomainNonRecursiveParallel_4(std::move(dpf_keys.first), sh_0);
        dpf.FullDomainNonRecursiveParallel_4(std::move(dpf_keys.second), sh_1);
        for (uint32_t i = 0; i < fde_size; i++) {
            res[i] = utils::Mod(sh_0[i] + sh_1[i], e);
        }
        result = DpfFullDomainCheck(alpha, beta, res, test_info.dbg_info.debug);

        dpf_keys.first.FreeDpfKey();
        dpf_keys.second.FreeDpfKey();
    }
    return result;
}

bool Test_FullDomainNonRecursiveParallel_8(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : utils::CreateSequence(9, 17)) {
        // Set DPF parameters
        DpfParameters            params(size, size, test_info.dbg_info);    // Input size equals to element size
        uint32_t                 n        = params.input_bitsize;
        uint32_t                 e        = params.element_bitsize;
        uint32_t                 fde_size = utils::Pow(2, n);
        DistributedPointFunction dpf(params);

        // Set input values
        uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta  = utils::Mod(tools::rng::SecureRng().Rand32(), e);

        // Generate keys
        std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeys(alpha, beta);

        // Evaluate Full Domain of DPF
        std::vector<uint32_t> sh_0(fde_size), sh_1(fde_size), res(fde_size);

        dpf.FullDomainNonRecursiveParallel_8(std::move(dpf_keys.first), sh_0);
        dpf.FullDomainNonRecursiveParallel_8(std::move(dpf_keys.second), sh_1);
        for (uint32_t i = 0; i < fde_size; i++) {
            res[i] = utils::Mod(sh_0[i] + sh_1[i], e);
        }
        result = DpfFullDomainCheck(alpha, beta, res, test_info.dbg_info.debug);

        dpf_keys.first.FreeDpfKey();
        dpf_keys.second.FreeDpfKey();
    }
    return result;
}

bool Test_FullDomainNonRecursive(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : utils::CreateSequence(2, 9)) {
        // Set DPF parameters
        DpfParameters            params(size, size, test_info.dbg_info);    // Input size equals to element size
        uint32_t                 n        = params.input_bitsize;
        uint32_t                 e        = params.element_bitsize;
        uint32_t                 fde_size = utils::Pow(2, n);
        DistributedPointFunction dpf(params);

        // Set input values
        uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta  = utils::Mod(tools::rng::SecureRng().Rand32(), e);

        // Generate keys
        std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeys(alpha, beta);

        // Evaluate Full Domain of DPF
        std::vector<uint32_t> sh_0(fde_size), sh_1(fde_size), res(fde_size);

        dpf.FullDomainNonRecursive(std::move(dpf_keys.first), sh_0);
        dpf.FullDomainNonRecursive(std::move(dpf_keys.second), sh_1);
        for (uint32_t i = 0; i < fde_size; i++) {
            res[i] = utils::Mod(sh_0[i] + sh_1[i], e);
        }
        result = DpfFullDomainCheck(alpha, beta, res, test_info.dbg_info.debug);

        dpf_keys.first.FreeDpfKey();
        dpf_keys.second.FreeDpfKey();
    }
    return result;
}

bool Test_FullDomainRecursive(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Set DPF parameters
        DpfParameters            params(size, size, test_info.dbg_info);
        uint32_t                 n        = params.input_bitsize;
        uint32_t                 e        = params.element_bitsize;
        uint32_t                 fde_size = utils::Pow(2, n);
        DistributedPointFunction dpf(params);

        // Set input values
        uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta  = utils::Mod(tools::rng::SecureRng().Rand32(), e);

        // Generate keys
        std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeys(alpha, beta);

        // Evaluate Full Domain of DPF
        std::vector<uint32_t> sh_0(fde_size), sh_1(fde_size), res(fde_size);

        dpf.FullDomainRecursive(std::move(dpf_keys.first), sh_0);
        dpf.FullDomainRecursive(std::move(dpf_keys.second), sh_1);
        for (uint32_t i = 0; i < fde_size; i++) {
            res[i] = utils::Mod(sh_0[i] + sh_1[i], e);
        }
        result = DpfFullDomainCheck(alpha, beta, res, test_info.dbg_info.debug);

        dpf_keys.first.FreeDpfKey();
        dpf_keys.second.FreeDpfKey();
    }
    return result;
}

bool Test_FullDomainNaive(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Set DPF parameters
        DpfParameters            params(size, size, test_info.dbg_info);
        uint32_t                 n        = params.input_bitsize;
        uint32_t                 e        = params.element_bitsize;
        uint32_t                 fde_size = utils::Pow(2, n);
        DistributedPointFunction dpf(params);

        // Set input values
        uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta  = utils::Mod(tools::rng::SecureRng().Rand32(), e);

        // Generate keys
        std::pair<DpfKey, DpfKey> dpf_keys = dpf.GenerateKeysNaive(alpha, beta);

        // Evaluate Full Domain of DPF
        std::vector<uint32_t> sh_0(fde_size), sh_1(fde_size), res(fde_size);

        dpf.FullDomainNaiveNaive(std::move(dpf_keys.first), sh_0);
        dpf.FullDomainNaiveNaive(std::move(dpf_keys.second), sh_1);
        for (uint32_t i = 0; i < fde_size; i++) {
            res[i] = utils::Mod(sh_0[i] + sh_1[i], e);
        }
        result = DpfFullDomainCheck(alpha, beta, res, test_info.dbg_info.debug);

        dpf_keys.first.FreeDpfKey();
        dpf_keys.second.FreeDpfKey();
    }
    return result;
}

}    // namespace test
}    // namespace dpf
}    // namespace fss
