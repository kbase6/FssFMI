/**
 * @file fsskey_io_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-10
 * @copyright Copyright (c) 2024
 * @brief FSS key I/O test implementation.
 */

#include "fsskey_io.hpp"

#include "../../tools/random_number_generator.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"

namespace {

const std::string kCurrentPath       = utils::GetCurrentDirectory();
const std::string kKeyIoPath         = kCurrentPath + "/data/test/keyio/";
const std::string kDpfKeyPathP0      = kKeyIoPath + "dpfkey_0";
const std::string kDpfKeyPathP1      = kKeyIoPath + "dpfkey_1";
const std::string kDpfNaiveKeyPathP0 = kKeyIoPath + "dpfkey_naive_0";
const std::string kDpfNaiveKeyPathP1 = kKeyIoPath + "dpfkey_naive_1";
const std::string kDcfKeyPathP0      = kKeyIoPath + "dcfkey_0";
const std::string kDcfKeyPathP1      = kKeyIoPath + "dcfkey_1";
const std::string kDdcfKeyPathP0     = kKeyIoPath + "ddcfkey_0";
const std::string kDdcfKeyPathP1     = kKeyIoPath + "ddcfkey_1";
const std::string kCompKeyPathP0     = kKeyIoPath + "comkey_0";
const std::string kCompKeyPathP1     = kKeyIoPath + "comkey_1";
const std::string kRankKeyPathP0     = kKeyIoPath + "rankkey_0";
const std::string kRankKeyPathP1     = kKeyIoPath + "rankkey_1";
const std::string kZtKeyPathP0       = kKeyIoPath + "ztkey_0";
const std::string kZtKeyPathP1       = kKeyIoPath + "ztkey_1";
const std::string kFmiKeyPathP0      = kKeyIoPath + "fmikey_0";
const std::string kFmiKeyPathP1      = kKeyIoPath + "fmikey_1";

}    // namespace

namespace fss {
namespace internal {
namespace test {

bool Test_DpfKeyIo(const TestInfo &test_info);
bool Test_DcfKeyIo(const TestInfo &test_info);
bool Test_DdcfKeyIo(const TestInfo &test_info);
bool Test_CompKeyIo(const TestInfo &test_info);
bool Test_RankKeyIo(const TestInfo &test_info);
bool Test_ZeroTestKeyIo(const TestInfo &test_info);
bool Test_FmiKeyIo(const TestInfo &test_info);

void Test_FssKeyIo(TestInfo &test_info) {
    std::vector<std::string> modes         = {"Key I/O unit tests", "DpfKeyIo", "DcfKeyIo", "DdcfKeyIo", "CompKeyIo", "RankKeyIo", "ZeroTestKeyIo", "FmiKeyIo"};
    uint32_t                 selected_mode = test_info.mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    utils::PrintText(utils::Logger::StrWithSep(modes[selected_mode - 1]));
    if (selected_mode == 1) {
        test_info.dbg_info.debug = false;
        utils::PrintTestResult("Test_DpfKeyIo", Test_DpfKeyIo(test_info));
        utils::PrintTestResult("Test_DcfKeyIo", Test_DcfKeyIo(test_info));
        utils::PrintTestResult("Test_DdcfKeyIo", Test_DdcfKeyIo(test_info));
        utils::PrintTestResult("Test_CompKeyIo", Test_CompKeyIo(test_info));
        utils::PrintTestResult("Test_RankKeyIo", Test_RankKeyIo(test_info));
        utils::PrintTestResult("Test_ZeroTestKeyIo", Test_ZeroTestKeyIo(test_info));
        utils::PrintTestResult("Test_FmiKeyIo", Test_FmiKeyIo(test_info));
    } else if (selected_mode == 2) {
        utils::PrintTestResult("Test_DpfKeyIo", Test_DpfKeyIo(test_info));
    } else if (selected_mode == 3) {
        utils::PrintTestResult("Test_DcfKeyIo", Test_DcfKeyIo(test_info));
    } else if (selected_mode == 4) {
        utils::PrintTestResult("Test_DdcfKeyIo", Test_DdcfKeyIo(test_info));
    } else if (selected_mode == 5) {
        utils::PrintTestResult("Test_CompKeyIo", Test_CompKeyIo(test_info));
    } else if (selected_mode == 6) {
        utils::PrintTestResult("Test_RankKeyIo", Test_RankKeyIo(test_info));
    } else if (selected_mode == 7) {
        utils::PrintTestResult("Test_ZeroTestKeyIo", Test_ZeroTestKeyIo(test_info));
    } else if (selected_mode == 8) {
        utils::PrintTestResult("Test_FmiKeyIo", Test_FmiKeyIo(test_info));
    }
    utils::PrintText(utils::kDash);
}

bool Test_DpfKeyIo(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Set DPF parameters
        dpf::DpfParameters            params(size, size, test_info.dbg_info);
        uint32_t                      n = params.input_bitsize;
        uint32_t                      e = params.element_bitsize;
        dpf::DistributedPointFunction dpf(params);
        FssKeyIo                      key_io(test_info.dbg_info.debug);
        dpf::DpfKey                   dpf_key_0, dpf_key_1;
        dpf::DpfKey                   dpf_key_0_naive, dpf_key_1_naive;

        // Set input values
        uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta  = utils::Mod(tools::rng::SecureRng().Rand32(), e);

        // Generate DPF keys
        std::pair<dpf::DpfKey, dpf::DpfKey> dpf_keys = dpf.GenerateKeys(alpha, beta);

        // Write and read DPF keys
        utils::Logger::DebugLog(LOCATION, "Write DPF key", test_info.dbg_info.debug);
        key_io.WriteDpfKeyToFile(kDpfKeyPathP0, dpf_keys.first);
        key_io.WriteDpfKeyToFile(kDpfKeyPathP1, dpf_keys.second);
        utils::Logger::DebugLog(LOCATION, "Read DPF key", test_info.dbg_info.debug);
        key_io.ReadDpfKeyFromFile(kDpfKeyPathP0, params, dpf_key_0);
        key_io.ReadDpfKeyFromFile(kDpfKeyPathP1, params, dpf_key_1);

        result &= dpf_key_0 == dpf_keys.first;
        result &= dpf_key_1 == dpf_keys.second;

        // Generate DPF keys (naive)
        std::pair<dpf::DpfKey, dpf::DpfKey> dpf_keys_naive = dpf.GenerateKeysNaive(alpha, beta);

        // Write and read DPF keys (naive)
        utils::Logger::DebugLog(LOCATION, "Write DPF key (naive)", test_info.dbg_info.debug);
        key_io.WriteDpfKeyToFile(kDpfNaiveKeyPathP0, dpf_keys_naive.first, true);
        key_io.WriteDpfKeyToFile(kDpfNaiveKeyPathP1, dpf_keys_naive.second, true);
        utils::Logger::DebugLog(LOCATION, "Read DPF key (naive)", test_info.dbg_info.debug);
        key_io.ReadDpfKeyFromFile(kDpfNaiveKeyPathP0, params, dpf_key_0_naive, true);
        key_io.ReadDpfKeyFromFile(kDpfNaiveKeyPathP1, params, dpf_key_1_naive, true);

        result &= dpf_key_0_naive == dpf_keys_naive.first;
        result &= dpf_key_1_naive == dpf_keys_naive.second;

        dpf_keys.first.FreeDpfKey();
        dpf_keys.second.FreeDpfKey();
        dpf_keys_naive.first.FreeDpfKey();
        dpf_keys_naive.second.FreeDpfKey();
        dpf_key_0.FreeDpfKey();
        dpf_key_1.FreeDpfKey();
        dpf_key_0_naive.FreeDpfKey();
        dpf_key_1_naive.FreeDpfKey();
    }
    return result;
}

bool Test_DcfKeyIo(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Set DCF parameters
        dcf::DcfParameters                 params(size, size, test_info.dbg_info);
        uint32_t                           n = params.input_bitsize;
        uint32_t                           e = params.element_bitsize;
        dcf::DistributedComparisonFunction dcf(params);
        FssKeyIo                           key_io(test_info.dbg_info.debug);
        dcf::DcfKey                        dcf_key_0, dcf_key_1;

        // Set input values
        uint32_t alpha = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta  = utils::Mod(tools::rng::SecureRng().Rand32(), e);

        // Generate DCF keys
        std::pair<dcf::DcfKey, dcf::DcfKey> dcf_keys = dcf.GenerateKeys(alpha, beta);

        // Write and read DCF keys
        utils::Logger::DebugLog(LOCATION, "Write DCF key", test_info.dbg_info.debug);
        key_io.WriteDcfKeyToFile(kDcfKeyPathP0, dcf_keys.first);
        key_io.WriteDcfKeyToFile(kDcfKeyPathP1, dcf_keys.second);
        utils::Logger::DebugLog(LOCATION, "Read DCF key", test_info.dbg_info.debug);
        key_io.ReadDcfKeyFromFile(kDcfKeyPathP0, n, dcf_key_0);
        key_io.ReadDcfKeyFromFile(kDcfKeyPathP1, n, dcf_key_1);

        result &= dcf_key_0 == dcf_keys.first;
        result &= dcf_key_1 == dcf_keys.second;

        dcf_keys.first.FreeDcfKey();
        dcf_keys.second.FreeDcfKey();
        dcf_key_0.FreeDcfKey();
        dcf_key_1.FreeDcfKey();
    }
    return result;
}

bool Test_DdcfKeyIo(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Set DDCF parameters
        ddcf::DdcfParameters                         params(size, size, test_info.dbg_info);
        uint32_t                                     n = params.input_bitsize;
        uint32_t                                     e = params.element_bitsize;
        fss::ddcf::DualDistributedComparisonFunction ddcf(params);
        FssKeyIo                                     key_io(test_info.dbg_info.debug);
        ddcf::DdcfKey                                ddcf_key_0, ddcf_key_1;

        // Set input values
        uint32_t alpha  = utils::Mod(tools::rng::SecureRng().Rand32(), n);
        uint32_t beta_1 = utils::Mod(tools::rng::SecureRng().Rand32(), e);
        uint32_t beta_2 = utils::Mod(tools::rng::SecureRng().Rand32(), e);

        // Generate DDCF keys
        std::pair<ddcf::DdcfKey, ddcf::DdcfKey> ddcf_keys = ddcf.GenerateKeys(alpha, beta_1, beta_2);

        // Write and read DDCF keys
        utils::Logger::DebugLog(LOCATION, "Write DDCF key", test_info.dbg_info.debug);
        key_io.WriteDdcfKeyToFile(kDdcfKeyPathP0, ddcf_keys.first);
        key_io.WriteDdcfKeyToFile(kDdcfKeyPathP1, ddcf_keys.second);
        utils::Logger::DebugLog(LOCATION, "Read DDCF key", test_info.dbg_info.debug);
        key_io.ReadDdcfKeyFromFile(kDdcfKeyPathP0, n, ddcf_key_0);
        key_io.ReadDdcfKeyFromFile(kDdcfKeyPathP1, n, ddcf_key_1);

        result &= ddcf_key_0 == ddcf_keys.first;
        result &= ddcf_key_1 == ddcf_keys.second;

        ddcf_keys.first.FreeDdcfKey();
        ddcf_keys.second.FreeDdcfKey();
        ddcf_key_0.FreeDdcfKey();
        ddcf_key_1.FreeDdcfKey();
    }
    return result;
}

bool Test_CompKeyIo(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Set COMP parameters
        comp::CompParameters    params(size, size, test_info.dbg_info);
        uint32_t                n = params.input_bitsize;
        comp::IntegerComparison comp(params);
        FssKeyIo                key_io(test_info.dbg_info.debug);
        comp::CompKey           comp_key_0, comp_key_1;

        // Generate COMP keys
        std::pair<comp::CompKey, comp::CompKey> comp_keys = comp.GenerateKeys();

        // Write and read COMP keys
        utils::Logger::DebugLog(LOCATION, "Write COMP key", test_info.dbg_info.debug);
        key_io.WriteCompKeyToFile(kCompKeyPathP0, comp_keys.first);
        key_io.WriteCompKeyToFile(kCompKeyPathP1, comp_keys.second);
        utils::Logger::DebugLog(LOCATION, "Read COMP key", test_info.dbg_info.debug);
        key_io.ReadCompKeyFromFile(kCompKeyPathP0, n, comp_key_0);
        key_io.ReadCompKeyFromFile(kCompKeyPathP1, n, comp_key_1);

        result &= comp_key_0 == comp_keys.first;
        result &= comp_key_1 == comp_keys.second;

        comp_keys.first.FreeCompKey();
        comp_keys.second.FreeCompKey();
        comp_key_0.FreeCompKey();
        comp_key_1.FreeCompKey();
    }
    return result;
}

bool Test_RankKeyIo(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Setting rank parameter
        rank::FssRankParameters params(size, test_info.dbg_info);
        rank::FssRank           rank(params);
        FssKeyIo                key_io(test_info.dbg_info.debug);
        rank::FssRankKey        rank_key_0, rank_key_1;

        // Gen algorithm of Rank
        std::pair<rank::FssRankKey, rank::FssRankKey> rank_keys = rank.GenerateKeys();

        // Write and read Rank keys
        utils::Logger::DebugLog(LOCATION, "Write Rank key", test_info.dbg_info.debug);
        key_io.WriteFssRankKeyToFile(kRankKeyPathP0, rank_keys.first);
        key_io.WriteFssRankKeyToFile(kRankKeyPathP1, rank_keys.second);
        utils::Logger::DebugLog(LOCATION, "Read Rank key", test_info.dbg_info.debug);
        key_io.ReadFssRankKeyFromFile(kRankKeyPathP0, params, rank_key_0);
        key_io.ReadFssRankKeyFromFile(kRankKeyPathP1, params, rank_key_1);

        result &= rank_key_0 == rank_keys.first;
        result &= rank_key_1 == rank_keys.second;

        rank_keys.first.FreeFssRankKey();
        rank_keys.second.FreeFssRankKey();
        rank_key_0.FreeFssRankKey();
        rank_key_1.FreeFssRankKey();
    }
    return result;
}

bool Test_ZeroTestKeyIo(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Setting zero test parameter
        zt::ZeroTestParameters params(size, size, test_info.dbg_info);
        zt::ZeroTest           zt(params);
        FssKeyIo               key_io(test_info.dbg_info.debug);
        zt::ZeroTestKey        zt_key_0, zt_key_1;

        // Generate Zero Test keys
        std::pair<zt::ZeroTestKey, zt::ZeroTestKey> zt_keys = zt.GenerateKeys();

        // Write and read Zero Test keys
        utils::Logger::DebugLog(LOCATION, "Write Zero Test key", test_info.dbg_info.debug);
        key_io.WriteZeroTestKeyToFile(kZtKeyPathP0, zt_keys.first);
        key_io.WriteZeroTestKeyToFile(kZtKeyPathP1, zt_keys.second);
        utils::Logger::DebugLog(LOCATION, "Read Zero Test key", test_info.dbg_info.debug);
        key_io.ReadZeroTestKeyFromFile(kZtKeyPathP0, params, zt_key_0);
        key_io.ReadZeroTestKeyFromFile(kZtKeyPathP1, params, zt_key_1);

        result &= zt_key_0 == zt_keys.first;
        result &= zt_key_1 == zt_keys.second;

        zt_keys.first.FreeZeroTestKey();
        zt_keys.second.FreeZeroTestKey();
        zt_key_0.FreeZeroTestKey();
        zt_key_1.FreeZeroTestKey();
    }
    return result;
}

bool Test_FmiKeyIo(const TestInfo &test_info) {
    bool result = true;
    for (const auto size : test_info.domain_size) {
        // Setting FM-Index parameter
        uint32_t              q = 4;
        fmi::FssFmiParameters params(size, q, test_info.dbg_info);
        uint32_t              qs = utils::Pow(2, q);
        fmi::FssFmi           fmi(params);
        FssKeyIo              key_io(test_info.dbg_info.debug);
        fmi::FssFmiKey        fmi_key_0, fmi_key_1;

        // Generate FM-Index keys
        std::pair<fmi::FssFmiKey, fmi::FssFmiKey> fmi_keys = fmi.GenerateKeys(qs - 1, qs);

        // Write and read FM-Index keys
        utils::Logger::DebugLog(LOCATION, "Write FM-Index key", test_info.dbg_info.debug);
        key_io.WriteFssFmiKeyToFile(kFmiKeyPathP0, fmi_keys.first);
        key_io.WriteFssFmiKeyToFile(kFmiKeyPathP1, fmi_keys.second);
        utils::Logger::DebugLog(LOCATION, "Read FM-Index key", test_info.dbg_info.debug);
        key_io.ReadFssFmiKeyFromFile(kFmiKeyPathP0, params, fmi_key_0);
        key_io.ReadFssFmiKeyFromFile(kFmiKeyPathP1, params, fmi_key_1);

        result &= fmi_key_0 == fmi_keys.first;
        result &= fmi_key_1 == fmi_keys.second;

        fmi_keys.first.FreeFssFmiKey();
        fmi_keys.second.FreeFssFmiKey();
        fmi_key_0.FreeFssFmiKey();
        fmi_key_1.FreeFssFmiKey();
    }
    return result;
}

}    // namespace test
}    // namespace internal
}    // namespace fss
