/**
 * @file zero_test_dpf_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-04-19
 * @copyright Copyright (c) 2024
 * @brief Zero Test DPF test implementation.
 */

#include "zero_test_dpf.hpp"

#include <thread>

#include "../../tools/random_number_generator.hpp"
#include "../../utils/file_io.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"
#include "../internal/fsskey_io.hpp"

namespace {

const std::string kCurrentPath         = utils::GetCurrentDirectory();
const std::string kTestZtPath          = kCurrentPath + "/data/test/zt/";
const std::string kZtKeyPath_P0        = kTestZtPath + "key_0";
const std::string kZtKeyPath_P1        = kTestZtPath + "key_1";
const std::string kZtBitKeyPath_P0     = kTestZtPath + "bit_key_0";
const std::string kZtBitKeyPath_P1     = kTestZtPath + "bit_key_1";
const std::string kZtDataPath_X        = kTestZtPath + "data";
const std::string kZtSharePath_X_P0    = kTestZtPath + "sh_0";
const std::string kZtSharePath_X_P1    = kTestZtPath + "sh_1";
const std::string kZtBitDataPath_X     = kTestZtPath + "bit_data";
const std::string kZtBitSharePath_X_P0 = kTestZtPath + "bit_sh_0";
const std::string kZtBitSharePath_X_P1 = kTestZtPath + "bit_sh_1";

constexpr uint32_t kNumOfElement = 2;

}    // namespace

namespace fss {
namespace zt {
namespace test {

bool Test_ZeroTestOffline(tools::secret_sharing::Party &party, const TestInfo &test_info);
bool Test_ZeroTestOneBitOffline(tools::secret_sharing::Party &party, const TestInfo &test_info);
bool Test_ZeroTestOnline(tools::secret_sharing::Party &party, const TestInfo &test_info);
bool Test_ZeroTestOneBitOnline(tools::secret_sharing::Party &party, const TestInfo &test_info);

void Test_ZeroTest(tools::secret_sharing::Party &party, TestInfo &test_info) {
    std::vector<std::string> modes         = {"Zero Test unit tests", "ZeroTestOffline", "ZeroTestOneBitOffline", "ZeroTestOnline", "ZeroTestOneBitOnline"};
    uint32_t                 selected_mode = test_info.mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    utils::PrintText(utils::Logger::StrWithSep(modes[selected_mode - 1]));
    if (selected_mode == 1) {
        test_info.dbg_info.debug = false;
        if (party.GetId() == 0) {
            utils::PrintTestResult("Test_ZeroTestOffline", Test_ZeroTestOffline(party, test_info));
            utils::PrintTestResult("Test_ZeroTestOneBitOffline", Test_ZeroTestOneBitOffline(party, test_info));
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        utils::PrintTestResult("Test_ZeroTestOnline", Test_ZeroTestOnline(party, test_info));
        utils::PrintTestResult("Test_ZeroTestOneBitOnline", Test_ZeroTestOneBitOnline(party, test_info));
    } else if (selected_mode == 2) {
        utils::PrintTestResult("Test_ZeroTestOffline", Test_ZeroTestOffline(party, test_info));
    } else if (selected_mode == 3) {
        utils::PrintTestResult("Test_ZeroTestOneBitOffline", Test_ZeroTestOneBitOffline(party, test_info));
    } else if (selected_mode == 4) {
        utils::PrintTestResult("Test_ZeroTestOnline", Test_ZeroTestOnline(party, test_info));
    } else if (selected_mode == 5) {
        utils::PrintTestResult("Test_ZeroTestOneBitOnline", Test_ZeroTestOneBitOnline(party, test_info));
    }
    utils::PrintText(utils::kDash);
}

bool Test_ZeroTestOffline(tools::secret_sharing::Party &party, const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Set parameters
        ZeroTestParameters                           params(size, size, test_info.dbg_info);
        tools::secret_sharing::AdditiveSecretSharing ss(size);
        utils::FileIo                                io;
        tools::secret_sharing::ShareHandler          sh;
        internal::FssKeyIo                           key_io(test_info.dbg_info.debug);
        ZeroTest                                     zt(params);

        // Generate input data
        std::vector<uint32_t>                                   x    = {0, 1};
        std::pair<std::vector<uint32_t>, std::vector<uint32_t>> x_sh = ss.Share(x);
        io.WriteVectorToFile(kZtDataPath_X, x);
        sh.ExportShare(kZtSharePath_X_P0, kZtSharePath_X_P1, x_sh);
        for (size_t i = 0; i < x.size(); i++) {
            utils::Logger::DebugLog(LOCATION, "x[" + std::to_string(i) + "]: " + std::to_string(x[i]) + " -> (" + std::to_string(x_sh.first[i]) + ", " + std::to_string(x_sh.second[i]) + ")", test_info.dbg_info.debug);
        }
        result &= (utils::Mod(x_sh.first[0] + x_sh.second[0], size) == 0) & (utils::Mod(x_sh.first[1] + x_sh.second[1], size) == 1);

        // Generate Zero Test key
        std::pair<ZeroTestKey, ZeroTestKey> zt_keys = zt.GenerateKeys();
        key_io.WriteZeroTestKeyToFile(kZtKeyPath_P0, zt_keys.first);
        key_io.WriteZeroTestKeyToFile(kZtKeyPath_P1, zt_keys.second);

        ZeroTestKey zt_key_0, zt_key_1;
        key_io.ReadZeroTestKeyFromFile(kZtKeyPath_P0, params, zt_key_0);
        key_io.ReadZeroTestKeyFromFile(kZtKeyPath_P1, params, zt_key_1);
        result &= (zt_keys.first == zt_key_0) & (zt_keys.second == zt_key_1);

        zt_keys.first.FreeZeroTestKey();
        zt_keys.second.FreeZeroTestKey();
    }
    return result;
}

bool Test_ZeroTestOneBitOffline(tools::secret_sharing::Party &party, const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        ZeroTestParameters                           params(size, 1, test_info.dbg_info);
        tools::secret_sharing::AdditiveSecretSharing ss(size);
        utils::FileIo                                io;
        tools::secret_sharing::ShareHandler          sh;
        internal::FssKeyIo                           key_io(test_info.dbg_info.debug);
        ZeroTest                                     zt(params);

        std::vector<uint32_t> x = {0, 1};

        io.WriteVectorToFile(kZtBitDataPath_X, x);

        std::pair<std::vector<uint32_t>, std::vector<uint32_t>> x_sh = ss.Share(x);
        sh.ExportShare(kZtBitSharePath_X_P0, kZtBitSharePath_X_P1, x_sh);
        for (size_t i = 0; i < x.size(); i++) {
            utils::Logger::DebugLog(LOCATION, "x[" + std::to_string(i) + "]: " + std::to_string(x[i]) + " -> (" + std::to_string(x_sh.first[i]) + ", " + std::to_string(x_sh.second[i]) + ")", test_info.dbg_info.debug);
        }
        std::pair<ZeroTestKey, ZeroTestKey> zt_keys = zt.GenerateKeys();

        key_io.WriteZeroTestKeyToFile(kZtBitKeyPath_P0, zt_keys.first);
        key_io.WriteZeroTestKeyToFile(kZtBitKeyPath_P1, zt_keys.second);

        result &= (utils::Mod(x_sh.first[0] + x_sh.second[0], size) == 0) & (utils::Mod(x_sh.first[1] + x_sh.second[1], size) == 1);

        zt_keys.first.FreeZeroTestKey();
        zt_keys.second.FreeZeroTestKey();
    }
    return result;
}

bool Test_ZeroTestOnline(tools::secret_sharing::Party &party, const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        ZeroTestParameters                           params(size, size, test_info.dbg_info);
        tools::secret_sharing::AdditiveSecretSharing ss(size);
        utils::FileIo                                io;
        tools::secret_sharing::ShareHandler          sh;
        internal::FssKeyIo                           key_io(test_info.dbg_info.debug);
        ZeroTest                                     zt(params);

        // Read Zero Test key from file
        ZeroTestKey zt_key;
        if (party.GetId() == 0) {
            key_io.ReadZeroTestKeyFromFile(kZtKeyPath_P0, params, zt_key);
        } else {
            key_io.ReadZeroTestKeyFromFile(kZtKeyPath_P1, params, zt_key);
        }

        // Read input data
        std::vector<uint32_t> x, x_0, x_1;
        io.ReadVectorFromFile(kZtDataPath_X, x);
        if (party.GetId() == 0) {
            io.ReadVectorFromFile(kZtSharePath_X_P0, x_0);
        } else {
            io.ReadVectorFromFile(kZtSharePath_X_P1, x_1);
        }

        // Reconst x + r_in
        party.StartCommunication();
        for (uint32_t i = 0; i < kNumOfElement; i++) {
            uint32_t e_0{0}, e_1{0}, xr_0{0}, xr_1{0};
            if (party.GetId() == 0) {
                xr_0 = utils::Mod(x_0[i] + zt_key.shr_in, size);
            } else {
                xr_1 = utils::Mod(x_1[i] + zt_key.shr_in, size);
            }
            uint32_t xr = ss.Reconst(party, xr_0, xr_1);
            utils::Logger::DebugLog(LOCATION, "xr: " + std::to_string(xr), test_info.dbg_info.debug);

            if (party.GetId() == 0) {
                e_0 = zt.EvaluateAt(zt_key, xr);
            } else {
                e_1 = zt.EvaluateAt(zt_key, xr);
            }
            uint32_t res = ss.Reconst(party, e_0, e_1);
            utils::Logger::DebugLog(LOCATION, "e: " + std::to_string(res), test_info.dbg_info.debug);
            result &= (res == (x[i] == 0 ? 1 : 0));
        }
    }
    return result;
}

bool Test_ZeroTestOneBitOnline(tools::secret_sharing::Party &party, const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        ZeroTestParameters                           params(size, 1, test_info.dbg_info);
        tools::secret_sharing::AdditiveSecretSharing ss(size);
        tools::secret_sharing::BooleanSecretSharing  ss_b;
        utils::FileIo                                io;
        tools::secret_sharing::ShareHandler          sh;
        internal::FssKeyIo                           key_io(test_info.dbg_info.debug);
        ZeroTest                                     zt(params);

        // Read Zero Test key from file
        ZeroTestKey zt_key;
        if (party.GetId() == 0) {
            key_io.ReadZeroTestKeyFromFile(kZtBitKeyPath_P0, params, zt_key);
        } else {
            key_io.ReadZeroTestKeyFromFile(kZtBitKeyPath_P1, params, zt_key);
        }

        // Read input data
        std::vector<uint32_t> x, x_0, x_1;
        io.ReadVectorFromFile(kZtBitDataPath_X, x);
        if (party.GetId() == 0) {
            io.ReadVectorFromFile(kZtBitSharePath_X_P0, x_0);
        } else {
            io.ReadVectorFromFile(kZtBitSharePath_X_P1, x_1);
        }

        // Reconst x + r_in
        party.StartCommunication();
        for (uint32_t i = 0; i < kNumOfElement; i++) {
            uint32_t e_0{0}, e_1{0}, xr_0{0}, xr_1{0};
            if (party.GetId() == 0) {
                xr_0 = utils::Mod(x_0[i] + zt_key.shr_in, size);
            } else {
                xr_1 = utils::Mod(x_1[i] + zt_key.shr_in, size);
            }
            uint32_t xr = ss.Reconst(party, xr_0, xr_1);
            utils::Logger::DebugLog(LOCATION, "xr: " + std::to_string(xr), test_info.dbg_info.debug);

            if (party.GetId() == 0) {
                e_0 = zt.EvaluateAt(zt_key, xr);
            } else {
                e_1 = zt.EvaluateAt(zt_key, xr);
            }
            uint32_t res = ss_b.Reconst(party, e_0, e_1);
            utils::Logger::DebugLog(LOCATION, "e: " + std::to_string(res), test_info.dbg_info.debug);
            result &= (res == (x[i] == 0 ? 1 : 0));
        }
        zt_key.FreeZeroTestKey();
    }
    return result;
}

}    // namespace test
}    // namespace zt
}    // namespace fss
