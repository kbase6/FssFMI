/**
 * @file tools_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Tools test implementation.
 */

#include "tools.hpp"

#include <sstream>
#include <thread>

#include "../utils/file_io.hpp"
#include "../utils/logger.hpp"
#include "../utils/utils.hpp"
#include "secret_sharing.hpp"

namespace {

const std::string kCurrentPath            = utils::GetCurrentDirectory();
const std::string kUtilsPath              = kCurrentPath + "/data/test/ss/";
const std::string kTestValPath            = kUtilsPath + "val";
const std::string kTestValPathP0          = kUtilsPath + "val_0";
const std::string kTestValPathP1          = kUtilsPath + "val_1";
const std::string kTestVecPath            = kUtilsPath + "vec";
const std::string kTestVecPathP0          = kUtilsPath + "vec_0";
const std::string kTestVecPathP1          = kUtilsPath + "vec_1";
const std::string kTestBoolPath           = kUtilsPath + "bool";
const std::string kTestBoolPathP0         = kUtilsPath + "bool_0";
const std::string kTestBoolPathP1         = kUtilsPath + "bool_1";
const std::string kTestBoolVecPath        = kUtilsPath + "bool_vec";
const std::string kTestBoolVecPathP0      = kUtilsPath + "bool_vec_0";
const std::string kTestBoolVecPathP1      = kUtilsPath + "bool_vec_1";
const std::string kTestBTPath             = kUtilsPath + "bt";
const std::string kTestBTPathP0           = kUtilsPath + "bt_0";
const std::string kTestBTPathP1           = kUtilsPath + "bt_1";
const std::string kTestBTBoolPath         = kUtilsPath + "btb";
const std::string kTestBTBoolPathP0       = kUtilsPath + "btb_0";
const std::string kTestBTBoolPathP1       = kUtilsPath + "btb_1";
const std::string kTestMultXPath          = kUtilsPath + "multx";
const std::string kTestMultXPathP0        = kUtilsPath + "multx_0";
const std::string kTestMultXPathP1        = kUtilsPath + "multx_1";
const std::string kTestMultYPath          = kUtilsPath + "multy";
const std::string kTestMultYPathP0        = kUtilsPath + "multy_0";
const std::string kTestMultYPathP1        = kUtilsPath + "multy_1";
const std::string kTestMultVecXPath       = kUtilsPath + "multvecx";
const std::string kTestMultVecXPathP0     = kUtilsPath + "multvecx_0";
const std::string kTestMultVecXPathP1     = kUtilsPath + "multvecx_1";
const std::string kTestMultVecYPath       = kUtilsPath + "multvecy";
const std::string kTestMultVecYPathP0     = kUtilsPath + "multvecy_0";
const std::string kTestMultVecYPathP1     = kUtilsPath + "multvecy_1";
const std::string kTestMultBoolXPath      = kUtilsPath + "multxb";
const std::string kTestMultBoolXPathP0    = kUtilsPath + "multxb_0";
const std::string kTestMultBoolXPathP1    = kUtilsPath + "multxb_1";
const std::string kTestMultBoolYPath      = kUtilsPath + "multyb";
const std::string kTestMultBoolYPathP0    = kUtilsPath + "multyb_0";
const std::string kTestMultBoolYPathP1    = kUtilsPath + "multyb_1";
const std::string kTestMultBoolVecXPath   = kUtilsPath + "multvecxb";
const std::string kTestMultBoolVecXPathP0 = kUtilsPath + "multvecxb_0";
const std::string kTestMultBoolVecXPathP1 = kUtilsPath + "multvecxb_1";
const std::string kTestMultBoolVecYPath   = kUtilsPath + "multvecyb";
const std::string kTestMultBoolVecYPathP0 = kUtilsPath + "multvecyb_0";
const std::string kTestMultBoolVecYPathP1 = kUtilsPath + "multvecyb_1";

}    // namespace

namespace tools {
namespace test {

bool Test_PartyComm(secret_sharing::Party &party, const bool debug);
bool Test_AdditiveSSOffline(secret_sharing::Party &party, const bool debug);
bool Test_BooleanSSOffline(secret_sharing::Party &party, const bool debug);
bool Test_AdditiveSSMultOffline(secret_sharing::Party &party, const bool debug);
bool Test_BooleanSSAndOrOffline(secret_sharing::Party &party, const bool debug);
bool Test_AdditiveSSOnline(secret_sharing::Party &party, const bool debug);
bool Test_BooleanSSOnline(secret_sharing::Party &party, const bool debug);
bool Test_AdditiveSSMultOnline(secret_sharing::Party &party, const bool debug);
bool Test_BooleanSSAndOrOnline(secret_sharing::Party &party, const bool debug);

void Test_SecretSharing(const comm::CommInfo &comm_info, const uint32_t mode, bool debug) {
    std::vector<std::string> modes         = {"SecretSharing unit tests", "PartyComm", "AdditiveSSOffline", "BooleanSSOffline", "AdditiveSSMultOffline", "BooleanSSAndOrOffline", "AdditiveSSOnline", "BooleanSSOnline", "AdditiveSSMultOnline", "BooleanSSAndOrOnline"};
    uint32_t                 selected_mode = mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    utils::PrintText(utils::Logger::StrWithSep(modes[selected_mode - 1]));
    secret_sharing::Party party(comm_info);
    if (selected_mode == 1) {
        debug = false;
        utils::PrintTestResult("Test_PartyComm", Test_PartyComm(party, debug));
        if (party.GetId() == 0) {
            utils::PrintTestResult("Test_AdditiveSSOffline", Test_AdditiveSSOffline(party, debug));
            utils::PrintTestResult("Test_BooleanSSOffline", Test_BooleanSSOffline(party, debug));
            utils::PrintTestResult("Test_AdditiveSSMultOffline", Test_AdditiveSSMultOffline(party, debug));
            utils::PrintTestResult("Test_BooleanSSAndOrOffline", Test_BooleanSSAndOrOffline(party, debug));
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        utils::PrintTestResult("Test_AdditiveSSOnline", Test_AdditiveSSOnline(party, debug));
        utils::PrintTestResult("Test_BooleanSSOnline", Test_BooleanSSOnline(party, debug));
        utils::PrintTestResult("Test_AdditiveSSMultOnline", Test_AdditiveSSMultOnline(party, debug));
        utils::PrintTestResult("Test_BooleanSSAndOrOnline", Test_BooleanSSAndOrOnline(party, debug));
    } else if (selected_mode == 2) {
        utils::PrintTestResult("Test_PartyComm", Test_PartyComm(party, debug));
    } else if (selected_mode == 3) {
        utils::PrintTestResult("Test_AdditiveSSOffline", Test_AdditiveSSOffline(party, debug));
    } else if (selected_mode == 4) {
        utils::PrintTestResult("Test_BooleanSSOffline", Test_BooleanSSOffline(party, debug));
    } else if (selected_mode == 5) {
        utils::PrintTestResult("Test_AdditiveSSMultOffline", Test_AdditiveSSMultOffline(party, debug));
    } else if (selected_mode == 6) {
        utils::PrintTestResult("Test_BooleanSSAndOrOffline", Test_BooleanSSAndOrOffline(party, debug));
    } else if (selected_mode == 7) {
        utils::PrintTestResult("Test_AdditiveSSOnline", Test_AdditiveSSOnline(party, debug));
    } else if (selected_mode == 8) {
        utils::PrintTestResult("Test_BooleanSSOnline", Test_BooleanSSOnline(party, debug));
    } else if (selected_mode == 9) {
        utils::PrintTestResult("Test_AdditiveSSMultOnline", Test_AdditiveSSMultOnline(party, debug));
    } else if (selected_mode == 10) {
        utils::PrintTestResult("Test_BooleanSSAndOrOnline", Test_BooleanSSAndOrOnline(party, debug));
    }
    utils::PrintText(utils::kDash);
}

bool Test_PartyComm(secret_sharing::Party &party, const bool debug) {
    bool result = true;
    party.StartCommunication();

    // Test SendRecv (value)
    uint32_t x_0(0), x_1(0);
    if (party.GetId() == 0) {
        x_0 = 5;
    } else {
        x_1 = 10;
    }
    party.SendRecv(x_0, x_1);
    utils::Logger::DebugLog(LOCATION, "x_0: " + std::to_string(x_0) + ", x_1: " + std::to_string(x_1), debug);
    result &= (x_0 == 5) & (x_1 == 10);

    // Test SendRecv (vector)
    std::vector<uint32_t> x_vec_0(5), x_vec_1(5);
    if (party.GetId() == 0) {
        x_vec_0 = utils::CreateSequence(5, 10);
    } else {
        x_vec_1 = utils::CreateSequence(10, 15);
    }
    party.SendRecv(x_vec_0, x_vec_1);
    utils::Logger::DebugLog(LOCATION, "x_vec_0: " + utils::VectorToStr(x_vec_0) + ", x_vec_1: " + utils::VectorToStr(x_vec_1), debug);
    result &= (x_vec_0 == utils::CreateSequence(5, 10)) & (x_vec_1 == utils::CreateSequence(10, 15));

    // Test SendRecv (array2)
    std::array<uint32_t, 2> x_arr2_0, x_arr2_1;
    if (party.GetId() == 0) {
        x_arr2_0 = {5, 10};
    } else {
        x_arr2_1 = {10, 15};
    }
    party.SendRecv(x_arr2_0, x_arr2_1);
    utils::Logger::DebugLog(LOCATION, "x_arr2_0: " + utils::ArrayToStr(x_arr2_0) + ", x_arr2_1: " + utils::ArrayToStr(x_arr2_1), debug);
    result &= (x_arr2_0[0] == 5) & (x_arr2_0[1] == 10) & (x_arr2_1[0] == 10) & (x_arr2_1[1] == 15);

    // Test SendRecv (array4)
    std::array<uint32_t, 4> x_arr4_0, x_arr4_1;
    if (party.GetId() == 0) {
        x_arr4_0 = {5, 10, 15, 20};
    } else {
        x_arr4_1 = {10, 15, 20, 25};
    }
    party.SendRecv(x_arr4_0, x_arr4_1);
    utils::Logger::DebugLog(LOCATION, "x_arr4_0: " + utils::ArrayToStr(x_arr4_0) + ", x_arr4_1: " + utils::ArrayToStr(x_arr4_1), debug);
    result &= (x_arr4_0[0] == 5) & (x_arr4_0[1] == 10) & (x_arr4_0[2] == 15) & (x_arr4_0[3] == 20) & (x_arr4_1[0] == 10) & (x_arr4_1[1] == 15) & (x_arr4_1[2] == 20) & (x_arr4_1[3] == 25);

    // Test total bytes sent
    uint32_t total_bytes = 0;
    total_bytes          = party.GetTotalBytesSent();
    utils::Logger::DebugLog(LOCATION, "Total bytes sent: " + std::to_string(total_bytes), debug);
    result &= (total_bytes > 0);

    return result;
}

bool Test_AdditiveSSOffline(secret_sharing::Party &party, const bool debug) {
    bool                                  result  = true;
    uint32_t                              bitsize = 5;
    secret_sharing::AdditiveSecretSharing ss_a(bitsize);
    utils::FileIo                         io;
    secret_sharing::ShareHandler          sh;

    // Test GenerateShare
    uint32_t                                                x        = 12;
    std::vector<uint32_t>                                   x_vec    = utils::CreateSequence(5, 10);
    std::pair<uint32_t, uint32_t>                           x_sh     = ss_a.Share(x);
    std::pair<std::vector<uint32_t>, std::vector<uint32_t>> x_vec_sh = ss_a.Share(x_vec);

    // Write share to file
    io.WriteValueToFile(kTestValPath, x);
    io.WriteVectorToFile(kTestVecPath, x_vec);
    sh.ExportShare(kTestValPathP0, kTestValPathP1, x_sh);
    sh.ExportShare(kTestVecPathP0, kTestVecPathP1, x_vec_sh);

    utils::Logger::DebugLog(LOCATION, "Share value: " + std::to_string(x) + " -> (" + std::to_string(x_sh.first) + ", " + std::to_string(x_sh.second) + ")", debug);
    for (size_t i = 0; i < x_vec.size(); i++) {
        utils::Logger::DebugLog(LOCATION, "Share vector: " + std::to_string(x_vec[i]) + " -> (" + std::to_string(x_vec_sh.first[i]) + ", " + std::to_string(x_vec_sh.second[i]) + ")", debug);
    }

    result &= (utils::Mod(x_sh.first + x_sh.second, bitsize) == x);
    for (size_t i = 0; i < x_vec.size(); i++) {
        result &= (utils::Mod(x_vec_sh.first[i] + x_vec_sh.second[i], bitsize) == x_vec[i]);
    }
    return result;
}

bool Test_AdditiveSSOnline(secret_sharing::Party &party, const bool debug) {
    bool                                  result  = true;
    uint32_t                              bitsize = 5;
    secret_sharing::AdditiveSecretSharing ss_a(bitsize);
    utils::FileIo                         io;
    secret_sharing::ShareHandler          sh;

    uint32_t              x     = 12;
    std::vector<uint32_t> x_vec = utils::CreateSequence(5, 10);

    party.StartCommunication();

    uint32_t              x_0(0), x_1(0);
    std::vector<uint32_t> x_vec_0(5), x_vec_1(5);

    // Load share from file
    if (party.GetId() == 0) {
        sh.LoadShare(kTestValPathP0, x_0);
        sh.LoadShare(kTestVecPathP0, x_vec_0);
        utils::Logger::DebugLog(LOCATION, "x_0: " + std::to_string(x_0), debug);
        utils::Logger::DebugLog(LOCATION, "x_vec_0: " + utils::VectorToStr(x_vec_0), debug);
    } else {
        sh.LoadShare(kTestValPathP1, x_1);
        sh.LoadShare(kTestVecPathP1, x_vec_1);
        utils::Logger::DebugLog(LOCATION, "x_1: " + std::to_string(x_1), debug);
        utils::Logger::DebugLog(LOCATION, "x_vec_1: " + utils::VectorToStr(x_vec_1), debug);
    }

    // Test Reconst
    uint32_t x_res = ss_a.Reconst(party, x_0, x_1);
    utils::Logger::DebugLog(LOCATION, "Reconst: " + std::to_string(x_res), debug);
    result &= (x_res == x);

    std::vector<uint32_t> x_vec_res(5);
    ss_a.Reconst(party, x_vec_0, x_vec_1, x_vec_res);
    for (size_t i = 0; i < x_vec.size(); i++) {
        utils::Logger::DebugLog(LOCATION, "Reconst : " + std::to_string(x_vec_res[i]), debug);
    }
    result &= (x_vec_res == x_vec);

    return result;
}

bool Test_BooleanSSOffline(secret_sharing::Party &party, const bool debug) {
    bool                                 result = true;
    secret_sharing::BooleanSecretSharing ss_b;
    utils::FileIo                        io;
    secret_sharing::ShareHandler         sh;

    // Test GenerateShare
    uint32_t                                                xb        = 0;
    std::vector<uint32_t>                                   xb_vec    = {0, 0, 1, 1, 1};
    std::pair<uint32_t, uint32_t>                           xb_sh     = ss_b.Share(xb);
    std::pair<std::vector<uint32_t>, std::vector<uint32_t>> xb_vec_sh = ss_b.Share(xb_vec);

    // Write share to file
    io.WriteValueToFile(kTestBoolPath, xb);
    io.WriteVectorToFile(kTestBoolVecPath, xb_vec);
    sh.ExportShare(kTestBoolPathP0, kTestBoolPathP1, xb_sh);
    sh.ExportShare(kTestBoolVecPathP0, kTestBoolVecPathP1, xb_vec_sh);

    utils::Logger::DebugLog(LOCATION, "Share bool value: " + std::to_string(xb) + " -> (" + std::to_string(xb_sh.first) + ", " + std::to_string(xb_sh.second) + ")", debug);
    for (size_t i = 0; i < xb_vec.size(); i++) {
        utils::Logger::DebugLog(LOCATION, "Share bool vector: " + std::to_string(xb_vec[i]) + " -> (" + std::to_string(xb_vec_sh.first[i]) + ", " + std::to_string(xb_vec_sh.second[i]) + ")", debug);
    }

    result &= (xb_sh.first ^ xb_sh.second) == xb;
    for (size_t i = 0; i < xb_vec.size(); i++) {
        result &= (xb_vec_sh.first[i] ^ xb_vec_sh.second[i]) == xb_vec[i];
    }
    return result;
}

bool Test_BooleanSSOnline(secret_sharing::Party &party, const bool debug) {
    bool                                 result = true;
    secret_sharing::BooleanSecretSharing ss_b;
    utils::FileIo                        io;
    secret_sharing::ShareHandler         sh;

    uint32_t              xb     = 0;
    std::vector<uint32_t> xb_vec = {0, 0, 1, 1, 1};

    party.StartCommunication();

    uint32_t              xb_0(0), xb_1(0);
    std::vector<uint32_t> xb_vec_0(5), xb_vec_1(5);

    if (party.GetId() == 0) {
        io.ReadValueFromFile(kTestBoolPathP0, xb_0);
        io.ReadVectorFromFile(kTestBoolVecPathP0, xb_vec_0);
        utils::Logger::DebugLog(LOCATION, "xb_0: " + std::to_string(xb_0), debug);
        utils::Logger::DebugLog(LOCATION, "xb_vec_0: " + utils::VectorToStr(xb_vec_0), debug);
    } else {
        io.ReadValueFromFile(kTestBoolPathP1, xb_1);
        io.ReadVectorFromFile(kTestBoolVecPathP1, xb_vec_1);
        utils::Logger::DebugLog(LOCATION, "xb_1: " + std::to_string(xb_1), debug);
        utils::Logger::DebugLog(LOCATION, "xb_vec_1: " + utils::VectorToStr(xb_vec_1), debug);
    }

    uint32_t xb_res = ss_b.Reconst(party, xb_0, xb_1);
    utils::Logger::DebugLog(LOCATION, "Reconst: " + std::to_string(xb_res), debug);
    result &= (xb_res == xb);

    std::vector<uint32_t> xb_vec_res(5);
    ss_b.Reconst(party, xb_vec_0, xb_vec_1, xb_vec_res);
    for (size_t i = 0; i < xb_vec.size(); i++) {
        utils::Logger::DebugLog(LOCATION, "Reconst : " + std::to_string(xb_vec_res[i]), debug);
    }
    result &= (xb_vec_res == xb_vec);
    return result;
}

bool Test_AdditiveSSMultOffline(secret_sharing::Party &party, const bool debug) {
    bool                                  result  = true;
    uint32_t                              bitsize = 5;
    secret_sharing::AdditiveSecretSharing ss_a(bitsize);
    utils::FileIo                         io;
    secret_sharing::ShareHandler          sh;

    uint32_t              num   = 4;
    uint32_t              x     = 3;
    uint32_t              y     = 3;
    std::vector<uint32_t> x_vec = utils::CreateVectorWithSameValue(2, num);
    std::vector<uint32_t> y_vec = utils::CreateSequence(0, num);
    secret_sharing::bts_t bt_vec(num);
    ss_a.GenerateBeaverTriples(num, bt_vec);
    std::pair<secret_sharing::bts_t, secret_sharing::bts_t> bt_vec_sh = ss_a.ShareBeaverTriples(bt_vec);
    std::pair<uint32_t, uint32_t>                           x_sh      = ss_a.Share(x);
    std::pair<uint32_t, uint32_t>                           y_sh      = ss_a.Share(y);
    std::pair<std::vector<uint32_t>, std::vector<uint32_t>> x_vec_sh  = ss_a.Share(x_vec);
    std::pair<std::vector<uint32_t>, std::vector<uint32_t>> y_vec_sh  = ss_a.Share(y_vec);

    io.WriteValueToFile(kTestMultXPath, x);
    io.WriteValueToFile(kTestMultYPath, y);
    io.WriteVectorToFile(kTestMultVecXPath, x_vec);
    io.WriteVectorToFile(kTestMultVecYPath, y_vec);
    sh.ExportShare(kTestMultXPathP0, kTestMultXPathP1, x_sh);
    sh.ExportShare(kTestMultYPathP0, kTestMultYPathP1, y_sh);
    sh.ExportShare(kTestMultVecXPathP0, kTestMultVecXPathP1, x_vec_sh);
    sh.ExportShare(kTestMultVecYPathP0, kTestMultVecYPathP1, y_vec_sh);
    sh.ExportBT(kTestBTPath, bt_vec);
    sh.ExportBTShare(kTestBTPathP0, kTestBTPathP1, bt_vec_sh);

    utils::Logger::DebugLog(LOCATION, "(x, y) -> (" + std::to_string(x) + ", " + std::to_string(y) + ")", debug);
    utils::Logger::DebugLog(LOCATION, "x_sh_0: " + std::to_string(x_sh.first) + ", x_sh_1: " + std::to_string(x_sh.second), debug);
    utils::Logger::DebugLog(LOCATION, "y_sh_0: " + std::to_string(y_sh.first) + ", y_sh_1: " + std::to_string(y_sh.second), debug);
    utils::Logger::DebugLog(LOCATION, "x_vec: " + utils::VectorToStr(x_vec), debug);
    utils::Logger::DebugLog(LOCATION, "y_vec: " + utils::VectorToStr(y_vec), debug);
    utils::Logger::DebugLog(LOCATION, "x_vec_sh_0: " + utils::VectorToStr(x_vec_sh.first) + ", x_vec_sh_1: " + utils::VectorToStr(x_vec_sh.second), debug);
    utils::Logger::DebugLog(LOCATION, "y_vec_sh_0: " + utils::VectorToStr(y_vec_sh.first) + ", y_vec_sh_1: " + utils::VectorToStr(y_vec_sh.second), debug);
    for (size_t i = 0; i < bt_vec.size(); i++) {
        utils::Logger::DebugLog(LOCATION, "Share of bt: " + bt_vec[i].ToStr() + " -> " + bt_vec_sh.first[i].ToStr(false) + ", " + bt_vec_sh.second[i].ToStr(false), debug);
    }
    result &= utils::Mod(x_sh.first + x_sh.second, bitsize) == x;
    return result;
}

bool Test_AdditiveSSMultOnline(secret_sharing::Party &party, const bool debug) {
    bool                                  result = true;
    secret_sharing::AdditiveSecretSharing ss_a(5);
    utils::FileIo                         io;
    secret_sharing::ShareHandler          sh;

    uint32_t              num   = 4;
    uint32_t              x     = 3;
    uint32_t              y     = 3;
    std::vector<uint32_t> x_vec = utils::CreateVectorWithSameValue(2, num);
    std::vector<uint32_t> y_vec = utils::CreateSequence(0, num);
    party.StartCommunication();

    uint32_t              x_0(0), x_1(0), y_0(0), y_1(0);
    std::vector<uint32_t> x_vec_0(num), x_vec_1(num), y_vec_0(num), y_vec_1(num);
    secret_sharing::bts_t bt_vec_0, bt_vec_1;

    if (party.GetId() == 0) {
        sh.LoadShare(kTestMultXPathP0, x_0);
        sh.LoadShare(kTestMultYPathP0, y_0);
        sh.LoadShare(kTestMultVecXPathP0, x_vec_0);
        sh.LoadShare(kTestMultVecYPathP0, y_vec_0);
        sh.LoadBTShare(kTestBTPathP0, bt_vec_0);

        utils::Logger::DebugLog(LOCATION, "x_0: " + std::to_string(x_0), debug);
        utils::Logger::DebugLog(LOCATION, "y_0: " + std::to_string(y_0), debug);
        utils::Logger::DebugLog(LOCATION, "x_vec_0: " + utils::VectorToStr(x_vec_0), debug);
        utils::Logger::DebugLog(LOCATION, "y_vec_0: " + utils::VectorToStr(y_vec_0), debug);
        for (size_t i = 0; i < bt_vec_0.size(); i++) {
            utils::Logger::DebugLog(LOCATION, "Share bt_0: " + bt_vec_0[i].ToStr(), debug);
        }
    } else {
        sh.LoadShare(kTestMultXPathP1, x_1);
        sh.LoadShare(kTestMultYPathP1, y_1);
        sh.LoadShare(kTestMultVecXPathP1, x_vec_1);
        sh.LoadShare(kTestMultVecYPathP1, y_vec_1);
        sh.LoadBTShare(kTestBTPathP1, bt_vec_1);

        utils::Logger::DebugLog(LOCATION, "x_1: " + std::to_string(x_1), debug);
        utils::Logger::DebugLog(LOCATION, "y_1: " + std::to_string(y_1), debug);
        utils::Logger::DebugLog(LOCATION, "x_vec_1: " + utils::VectorToStr(x_vec_1), debug);
        utils::Logger::DebugLog(LOCATION, "y_vec_1: " + utils::VectorToStr(y_vec_1), debug);
        for (size_t i = 0; i < bt_vec_1.size(); i++) {
            utils::Logger::DebugLog(LOCATION, "Share bt_1: " + bt_vec_1[i].ToStr(), debug);
        }
    }

    uint32_t              z_0(0), z_1(0);
    std::vector<uint32_t> z_vec_0(num), z_vec_1(num);
    std::vector<uint32_t> z_vec_res(num);

    if (party.GetId() == 0) {
        z_0 = ss_a.Mult(party, bt_vec_0[0], x_0, y_0);
        ss_a.Mult(party, bt_vec_0, x_vec_0, y_vec_0, z_vec_0);
    } else {
        z_1 = ss_a.Mult(party, bt_vec_1[0], x_1, y_1);
        ss_a.Mult(party, bt_vec_1, x_vec_1, y_vec_1, z_vec_1);
    }

    uint32_t z_res = ss_a.Reconst(party, z_0, z_1);
    ss_a.Reconst(party, z_vec_0, z_vec_1, z_vec_res);

    utils::Logger::DebugLog(LOCATION, "Reconst: " + std::to_string(z_res), debug);
    utils::Logger::DebugLog(LOCATION, "Reconst: " + utils::VectorToStr(z_vec_res), debug);

    result &= (z_res == (x * y));
    for (size_t i = 0; i < z_vec_res.size(); i++) {
        result &= (z_vec_res[i] == (x_vec[i] * y_vec[i]));
    }
    return result;
}

bool Test_BooleanSSAndOrOffline(secret_sharing::Party &party, const bool debug) {
    bool                                 result = true;
    secret_sharing::BooleanSecretSharing ss_b;
    utils::FileIo                        io;
    secret_sharing::ShareHandler         sh;

    uint32_t              num    = 4;
    uint32_t              xb     = 0;
    uint32_t              yb     = 1;
    std::vector<uint32_t> xb_vec = {0, 0, 1, 1};
    std::vector<uint32_t> yb_vec = {0, 1, 0, 1};

    io.WriteValueToFile(kTestMultBoolXPath, xb);
    io.WriteValueToFile(kTestMultBoolYPath, yb);
    io.WriteVectorToFile(kTestMultBoolVecXPath, xb_vec);
    io.WriteVectorToFile(kTestMultBoolVecYPath, yb_vec);

    secret_sharing::bts_t btb_vec(num);
    ss_b.GenerateBeaverTriples(num, btb_vec);
    std::pair<secret_sharing::bts_t, secret_sharing::bts_t> btb_vec_sh = ss_b.ShareBeaverTriples(btb_vec);

    std::pair<uint32_t, uint32_t>                           xb_sh     = ss_b.Share(xb);
    std::pair<uint32_t, uint32_t>                           yb_sh     = ss_b.Share(yb);
    std::pair<std::vector<uint32_t>, std::vector<uint32_t>> xb_vec_sh = ss_b.Share(xb_vec);
    std::pair<std::vector<uint32_t>, std::vector<uint32_t>> yb_vec_sh = ss_b.Share(yb_vec);

    sh.ExportBT(kTestBTBoolPath, btb_vec);
    sh.ExportShare(kTestMultBoolXPathP0, kTestMultBoolXPathP1, xb_sh);
    sh.ExportShare(kTestMultBoolYPathP0, kTestMultBoolYPathP1, yb_sh);
    sh.ExportShare(kTestMultBoolVecXPathP0, kTestMultBoolVecXPathP1, xb_vec_sh);
    sh.ExportShare(kTestMultBoolVecYPathP0, kTestMultBoolVecYPathP1, yb_vec_sh);
    sh.ExportBTShare(kTestBTBoolPathP0, kTestBTBoolPathP1, btb_vec_sh);

    utils::Logger::DebugLog(LOCATION, "(xb, yb) -> (" + std::to_string(xb) + ", " + std::to_string(yb) + ")", debug);
    utils::Logger::DebugLog(LOCATION, "xb_vec: " + utils::VectorToStr(xb_vec), debug);
    utils::Logger::DebugLog(LOCATION, "yb_vec: " + utils::VectorToStr(yb_vec), debug);
    for (size_t i = 0; i < btb_vec.size(); i++) {
        utils::Logger::DebugLog(LOCATION, "Share of btb: " + btb_vec[i].ToStr() + " -> " + btb_vec_sh.first[i].ToStr(false) + ", " + btb_vec_sh.second[i].ToStr(false), debug);
    }

    result &= (xb_sh.first ^ xb_sh.second) == xb;
    result &= (yb_sh.first ^ yb_sh.second) == yb;
    for (size_t i = 0; i < xb_vec.size(); i++) {
        result &= (xb_vec_sh.first[i] ^ xb_vec_sh.second[i]) == xb_vec[i];
        result &= (yb_vec_sh.first[i] ^ yb_vec_sh.second[i]) == yb_vec[i];
    }
    return result;
}

bool Test_BooleanSSAndOrOnline(secret_sharing::Party &party, const bool debug) {
    bool                                 result = true;
    secret_sharing::BooleanSecretSharing ss_b;
    utils::FileIo                        io;
    secret_sharing::ShareHandler         sh;

    uint32_t              num    = 4;
    uint32_t              xb     = 0;
    uint32_t              yb     = 1;
    std::vector<uint32_t> xb_vec = {0, 0, 1, 1};
    std::vector<uint32_t> yb_vec = {0, 1, 0, 1};
    party.StartCommunication();

    uint32_t              xb_0(0), xb_1(0), yb_0(0), yb_1(0);
    std::vector<uint32_t> xb_vec_0(num), xb_vec_1(num), yb_vec_0(num), yb_vec_1(num);
    secret_sharing::bts_t btb_vec_0, btb_vec_1;

    if (party.GetId() == 0) {
        sh.LoadShare(kTestMultBoolXPathP0, xb_0);
        sh.LoadShare(kTestMultBoolYPathP0, yb_0);
        sh.LoadShare(kTestMultBoolVecXPathP0, xb_vec_0);
        sh.LoadShare(kTestMultBoolVecYPathP0, yb_vec_0);
        sh.LoadBTShare(kTestBTBoolPathP0, btb_vec_0);

        utils::Logger::DebugLog(LOCATION, "xb_0: " + std::to_string(xb_0), debug);
        utils::Logger::DebugLog(LOCATION, "yb_0: " + std::to_string(yb_0), debug);
        utils::Logger::DebugLog(LOCATION, "xb_vec_0: " + utils::VectorToStr(xb_vec_0), debug);
        utils::Logger::DebugLog(LOCATION, "yb_vec_0: " + utils::VectorToStr(yb_vec_0), debug);
        for (size_t i = 0; i < btb_vec_0.size(); i++) {
            utils::Logger::DebugLog(LOCATION, "Share btb_0: " + btb_vec_0[i].ToStr(), debug);
        }
    } else {
        sh.LoadShare(kTestMultBoolXPathP1, xb_1);
        sh.LoadShare(kTestMultBoolYPathP1, yb_1);
        sh.LoadShare(kTestMultBoolVecXPathP1, xb_vec_1);
        sh.LoadShare(kTestMultBoolVecYPathP1, yb_vec_1);
        sh.LoadBTShare(kTestBTBoolPathP1, btb_vec_1);

        utils::Logger::DebugLog(LOCATION, "xb_1: " + std::to_string(xb_1), debug);
        utils::Logger::DebugLog(LOCATION, "yb_1: " + std::to_string(yb_1), debug);
        utils::Logger::DebugLog(LOCATION, "xb_vec_1: " + utils::VectorToStr(xb_vec_1), debug);
        utils::Logger::DebugLog(LOCATION, "yb_vec_1: " + utils::VectorToStr(yb_vec_1), debug);
        for (size_t i = 0; i < btb_vec_1.size(); i++) {
            utils::Logger::DebugLog(LOCATION, "Share btb_1: " + btb_vec_1[i].ToStr(), debug);
        }
    }

    std::vector<uint32_t> z_vec_0(num), z_vec_1(num);
    uint32_t              zb_0(0), zb_1(0);
    std::vector<uint32_t> zb_vec_0(num), zb_vec_1(num);
    uint32_t              zbor_0(0), zbor_1(0);
    std::vector<uint32_t> zbor_vec_0(num), zbor_vec_1(num);
    std::vector<uint32_t> zb_vec_res(num);
    std::vector<uint32_t> zbor_vec_res(num);

    if (party.GetId() == 0) {
        zb_0 = ss_b.And(party, btb_vec_0[0], xb_0, yb_0);
        ss_b.And(party, btb_vec_0, xb_vec_0, yb_vec_0, zb_vec_0);
        zbor_0 = ss_b.Or(party, btb_vec_0[0], xb_0, yb_0);
        ss_b.Or(party, btb_vec_0, xb_vec_0, yb_vec_0, zbor_vec_0);
    } else {
        zb_1 = ss_b.And(party, btb_vec_1[0], xb_1, yb_1);
        ss_b.And(party, btb_vec_1, xb_vec_1, yb_vec_1, zb_vec_1);
        zbor_1 = ss_b.Or(party, btb_vec_1[0], xb_1, yb_1);
        ss_b.Or(party, btb_vec_1, xb_vec_1, yb_vec_1, zbor_vec_1);
    }

    uint32_t zb_res = ss_b.Reconst(party, zb_0, zb_1);
    ss_b.Reconst(party, zb_vec_0, zb_vec_1, zb_vec_res);
    uint32_t zbor_res = ss_b.Reconst(party, zbor_0, zbor_1);
    ss_b.Reconst(party, zbor_vec_0, zbor_vec_1, zbor_vec_res);

    utils::Logger::DebugLog(LOCATION, "Reconst: " + std::to_string(zb_res), debug);
    utils::Logger::DebugLog(LOCATION, "Reconst: " + utils::VectorToStr(zb_vec_res), debug);
    utils::Logger::DebugLog(LOCATION, "Reconst: " + std::to_string(zbor_res), debug);
    utils::Logger::DebugLog(LOCATION, "Reconst: " + utils::VectorToStr(zbor_vec_res), debug);

    result &= (zb_res == (xb & yb));
    for (size_t i = 0; i < zb_vec_res.size(); i++) {
        result &= (zb_vec_res[i] == (xb_vec[i] & yb_vec[i]));
    }
    result &= (zbor_res == (xb | yb));
    for (size_t i = 0; i < zbor_vec_res.size(); i++) {
        result &= (zbor_vec_res[i] == (xb_vec[i] | yb_vec[i]));
    }
    return result;
}

}    // namespace test
}    // namespace tools
