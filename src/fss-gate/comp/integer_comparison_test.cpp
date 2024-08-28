/**
 * @file integer_comparison_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-02-05
 * @copyright Copyright (c) 2024
 * @brief Integer Comparison test implementation.
 */

#include "integer_comparison.hpp"

#include "../../tools/random_number_generator.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"
#include "../internal/fsskey_io.hpp"

namespace {

const std::string kCurrentPath      = utils::GetCurrentDirectory();
const std::string kTestCompPath     = kCurrentPath + "/data/test/comp/";
const std::string kCompKeyPathP0    = kTestCompPath + "key_0";
const std::string kCompKeyPathP1    = kTestCompPath + "key_1";
const std::string kCompDataXPath    = kTestCompPath + "data_x";
const std::string kCompDataYPath    = kTestCompPath + "data_y";
const std::string kCompShareXPathP0 = kTestCompPath + "shx_0";
const std::string kCompShareXPathP1 = kTestCompPath + "shx_1";
const std::string kCompShareYPathP0 = kTestCompPath + "shy_0";
const std::string kCompShareYPathP1 = kTestCompPath + "shy_1";

constexpr int kNumOfElement = 32;

}    // namespace

namespace fss {
namespace comp {
namespace test {

void Test_Comp(tools::secret_sharing::Party &party, const TestInfo &test_info) {
    // Setting comparison parameter
    int                                          n = 5;
    int                                          e = 5;
    comp::CompParameters                         params(n, e, test_info.dbg_info);
    uint32_t                                     half_domain_size = utils::Pow(2, n - 1);
    tools::secret_sharing::AdditiveSecretSharing ss(e);
    utils::FileIo                                io;
    tools::secret_sharing::ShareHandler          sh;
    internal::FssKeyIo                           key_io(true);
    comp::IntegerComparison                      comp(params);

    std::vector<std::string> modes         = {"Generate share of data.", "Generate COMP key.", "Execute Eval^{Comp} algorithm"};
    int                      selected_mode = test_info.mode;
    if (selected_mode < 1 || selected_mode > static_cast<int>(modes.size())) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    utils::Logger::InfoLog(LOCATION, "COMP: (input size, element size) = (" + std::to_string(n) + ", " + std::to_string(e) + ")");
    if (selected_mode == 1) {
        std::vector<uint32_t> x, y;
        x = utils::CreateSequence(0, kNumOfElement);
        y = utils::CreateVectorWithSameValue(5, kNumOfElement);
        io.WriteVectorToFile(kCompDataXPath, x);
        io.WriteVectorToFile(kCompDataYPath, y);

        std::pair<std::vector<uint32_t>, std::vector<uint32_t>> x_sh = ss.Share(x);
        std::pair<std::vector<uint32_t>, std::vector<uint32_t>> y_sh = ss.Share(y);

        sh.ExportShare(kCompShareXPathP0, kCompShareXPathP1, x_sh);
        sh.ExportShare(kCompShareYPathP0, kCompShareYPathP1, y_sh);

        for (size_t i = 0; i < x.size(); i++) {
            utils::Logger::InfoLog(LOCATION, "x[" + std::to_string(i) + "]: " + std::to_string(x[i]) + " -> (" + std::to_string(x_sh.first[i]) + ", " + std::to_string(x_sh.second[i]) + ")");
        }
        for (size_t i = 0; i < x.size(); i++) {
            utils::Logger::InfoLog(LOCATION, "y[" + std::to_string(i) + "]: " + std::to_string(y[i]) + " -> (" + std::to_string(y_sh.first[i]) + ", " + std::to_string(y_sh.second[i]) + ")");
        }
    } else if (selected_mode == 2) {
        // Gen algorithm of COMP
        std::pair<comp::CompKey, comp::CompKey> comp_keys = comp.GenerateKeys();

        utils::Logger::InfoLog(LOCATION, "Write COMP key");
        key_io.WriteCompKeyToFile(kCompKeyPathP0, comp_keys.first);
        key_io.WriteCompKeyToFile(kCompKeyPathP1, comp_keys.second);

        comp_keys.first.PrintCompKey(test_info.dbg_info.debug);
        comp_keys.second.PrintCompKey(test_info.dbg_info.debug);
    } else {
        // Read Comp key
        utils::Logger::InfoLog(LOCATION, "Read Comp key");
        CompKey comp_key;
        if (party.GetId() == 0) {
            key_io.ReadCompKeyFromFile(kCompKeyPathP0, n, comp_key);
        } else {
            key_io.ReadCompKeyFromFile(kCompKeyPathP1, n, comp_key);
        }

        // Read input data
        utils::Logger::InfoLog(LOCATION, "Read Input data");
        std::vector<uint32_t> x(kNumOfElement), y(kNumOfElement);
        std::vector<uint32_t> x_0(kNumOfElement), x_1(kNumOfElement), y_0(kNumOfElement), y_1(kNumOfElement);
        io.ReadVectorFromFile(kCompDataXPath, x);
        io.ReadVectorFromFile(kCompDataYPath, y);
        if (party.GetId() == 0) {
            sh.LoadShare(kCompShareXPathP0, x_0);
            sh.LoadShare(kCompShareYPathP0, y_0);
        } else {
            sh.LoadShare(kCompShareXPathP1, x_1);
            sh.LoadShare(kCompShareYPathP1, y_1);
        }

        // Reconst x+r1_in, y+r2_in
        for (int i = 0; i < kNumOfElement; i++) {
            if (party.GetId() == 0) {
                x_0[i] = utils::Mod(x_0[i] + comp_key.shr1_in, e);
                y_0[i] = utils::Mod(y_0[i] + comp_key.shr2_in, e);
            } else {
                x_1[i] = utils::Mod(x_1[i] + comp_key.shr1_in, e);
                y_1[i] = utils::Mod(y_1[i] + comp_key.shr2_in, e);
            }
        }

        party.StartCommunication();

        std::vector<uint32_t> xr(kNumOfElement), yr(kNumOfElement);
        ss.Reconst(party, x_0, x_1, x);
        ss.Reconst(party, y_0, y_1, y);

        std::vector<uint32_t> sh_res_0(kNumOfElement), sh_res_1(kNumOfElement);

        for (int i = 0; i < kNumOfElement; i++) {
            if (utils::Abs(utils::To2Complement(x[i], e)) + utils::Abs(utils::To2Complement(y[i], e)) < half_domain_size) {

            } else {
                utils::Logger::ErrorLog(LOCATION, "Over half domain size: " + std::to_string(half_domain_size) + " -> (x, y)=(" + std::to_string(x[i]) + ", " + std::to_string(y[i]) + "), |x|=" + std::to_string(utils::Abs(utils::To2Complement(x[i], e))) + ", |y|=" + std::to_string(utils::Abs(utils::To2Complement(y[i], e))));
                utils::Logger::ErrorLog(LOCATION, "Over half domain size: " + std::to_string(half_domain_size));
            }
            if (party.GetId() == 0) {
                sh_res_0[i] = utils::Mod(comp.Evaluate(comp_key, xr[i], yr[i]) - comp_key.shr_out, n);
            } else {
                sh_res_1[i] = utils::Mod(comp.Evaluate(comp_key, xr[i], yr[i]) - comp_key.shr_out, n);
            }
            uint32_t res = ss.Reconst(party, sh_res_0[i], sh_res_1[i]);
            utils::Logger::InfoLog(LOCATION, "(x, y)=(" + std::to_string(x[i]) + ", " + std::to_string(y[i]) + ") -> " + std::to_string(res) + " (=" + std::to_string(sh_res_0[i]) + "+" + std::to_string(sh_res_1[i]) + ")");
            if (utils::To2Complement(x[i], e) < utils::To2Complement(y[i], e)) {
                utils::PrintValidity(__FUNCTION__, res, 1, test_info.dbg_info.debug);
            } else {
                utils::PrintValidity(__FUNCTION__, res, 0, test_info.dbg_info.debug);
            }
        }
        // std::vector<uint32_t> res = ss.Reconst(party, sh_res_0, sh_res_1);
        // for (int i = 0; i < kNumOfElement; i++) {
        //     utils::Logger::InfoLog(LOCATION, "(x, y)=(" + std::to_string(x[i]) + ", " + std::to_string(y[i]) + ") -> " + std::to_string(res[i]) + " (=" + std::to_string(sh_res_0[i]) + "+" + std::to_string(sh_res_1[i]) + ")");
        //     utils::PrintValidity(__FUNCTION__, "Comp(x, y)", (res[i] != 0) == (x[i] < y[i]));
        // }
    }
}

}    // namespace test
}    // namespace comp
}    // namespace fss
