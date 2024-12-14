/**
 * @file fssgate.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-12-11
 * @copyright Copyright (c) 2024
 * @brief
 */

#include "fssgate.hpp"

#include <sdsl/csa_wt.hpp>
#include <sdsl/suffix_arrays.hpp>

#include "../fss-gate/internal/fsskey_io.hpp"
#include "../utils/file_io.hpp"
#include "../utils/logger.hpp"
#include "../utils/utils.hpp"

namespace {

const std::string kCurrentPath  = utils::GetCurrentDirectory();
const std::string kTestZtPath   = kCurrentPath + "/data/zt/";
const std::string kZtKeyPath_P0 = kTestZtPath + "key_p0";
const std::string kZtKeyPath_P1 = kTestZtPath + "key_p1";

const std::string kTestEqPath   = kCurrentPath + "/data/eq/";
const std::string kEqKeyPath_P0 = kTestEqPath + "key_p0";
const std::string kEqKeyPath_P1 = kTestEqPath + "key_p1";

const std::string kTestCompPath   = kCurrentPath + "/data/comp/";
const std::string kCompKeyPath_P0 = kTestCompPath + "key_p0";
const std::string kCompKeyPath_P1 = kTestCompPath + "key_p1";

const std::string kFMIPath        = kCurrentPath + "/data/fmi/";
const std::string kFMIBTPath_F    = kFMIPath + "btf";
const std::string kFMIBTPath_F_P0 = kFMIPath + "btf_p0";
const std::string kFMIBTPath_F_P1 = kFMIPath + "btf_p1";
const std::string kFMIBTPath_G    = kFMIPath + "btg";
const std::string kFMIBTPath_G_P0 = kFMIPath + "btg_p0";
const std::string kFMIBTPath_G_P1 = kFMIPath + "btg_p1";
const std::string kFMIKeyPath_P0  = kFMIPath + "key_p0";
const std::string kFMIKeyPath_P1  = kFMIPath + "key_p1";
const std::string kFMIDBPath      = kFMIPath + "db";
const std::string kFMIBWTPath     = kFMIPath + "bwt";

fss::DebugInfo          dbg_info = fss::DebugInfo();
fss::internal::FssKeyIo key_io;

using bts_t = tools::secret_sharing::bts_t;

constexpr uint32_t kMaxQuerySize = 7;

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

}    // namespace

namespace fss {

void ZeroTestSetup(const uint32_t bitsize) {
    zt::ZeroTestParameters params(bitsize, bitsize, dbg_info);
    zt::ZeroTest           zt(params);

    // Generate keys
    std::pair<zt::ZeroTestKey, zt::ZeroTestKey> zt_keys = zt.GenerateKeys();

    key_io.WriteZeroTestKeyToFile(kZtKeyPath_P0, zt_keys.first);
    key_io.WriteZeroTestKeyToFile(kZtKeyPath_P1, zt_keys.second);

    utils::Logger::InfoLog(LOCATION, "Zero Test keys have been generated.");

    zt_keys.first.FreeZeroTestKey();
    zt_keys.second.FreeZeroTestKey();
}

void EqualitySetup(const uint32_t bitsize) {
    zt::ZeroTestParameters params(bitsize, bitsize, dbg_info);
    zt::ZeroTest           zt(params);

    // Generate keys
    std::pair<zt::ZeroTestKey, zt::ZeroTestKey> zt_keys = zt.GenerateKeys();

    key_io.WriteZeroTestKeyToFile(kEqKeyPath_P0, zt_keys.first);
    key_io.WriteZeroTestKeyToFile(kEqKeyPath_P1, zt_keys.second);

    utils::Logger::InfoLog(LOCATION, "Equality Test keys have been generated.");

    zt_keys.first.FreeZeroTestKey();
    zt_keys.second.FreeZeroTestKey();
}

void CompareSetup(const uint32_t bitsize) {
    comp::CompParameters    params(bitsize, bitsize, dbg_info);
    comp::IntegerComparison comp(params);

    // Generate keys
    std::pair<comp::CompKey, comp::CompKey> comp_keys = comp.GenerateKeys();

    key_io.WriteCompKeyToFile(kCompKeyPath_P0, comp_keys.first);
    key_io.WriteCompKeyToFile(kCompKeyPath_P1, comp_keys.second);

    utils::Logger::InfoLog(LOCATION, "Comparison keys have been generated.");

    comp_keys.first.FreeCompKey();
    comp_keys.second.FreeCompKey();
}

void FMISearchSetup(const uint32_t bitsize, std::vector<uint32_t> &database) {
    fmi::FssFmiParameters                        params(bitsize, kMaxQuerySize, dbg_info);
    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);
    tools::secret_sharing::ShareHandler          sh;
    utils::FileIo                                io;
    uint32_t                                     qs = params.query_size;
    fmi::FssFmi                                  fss_fmi(params);

    // Construct the BWT from the input database
    io.WriteVectorToFile(kFMIDBPath, database);
    std::reverse(database.begin(), database.end());    // To find LPM, we need to reverse the text
    std::string bwt = ConstructBwtFromVector(utils::VectorToStr(database, ""));
    io.WriteStringToFile(kFMIBWTPath, bwt);

    utils::Logger::InfoLog(LOCATION, "BWT has been constructed.");

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

    utils::Logger::InfoLog(LOCATION, "Beaver triples have been generated.");

    // Generate keys
    std::pair<fmi::FssFmiKey, fmi::FssFmiKey> fmi_keys = fss_fmi.GenerateKeys(qs - 1, qs);

    key_io.WriteFssFmiKeyToFile(kFMIKeyPath_P0, fmi_keys.first);
    key_io.WriteFssFmiKeyToFile(kFMIKeyPath_P1, fmi_keys.second);

    utils::Logger::InfoLog(LOCATION, "FMI Search keys have been generated.");

    fmi_keys.first.FreeFssFmiKey();
    fmi_keys.second.FreeFssFmiKey();
}

uint32_t ZeroTest(tools::secret_sharing::Party &party, const uint32_t x, const uint32_t bitsize) {
    zt::ZeroTestParameters                       params(bitsize, bitsize, dbg_info);
    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);
    zt::ZeroTest                                 zt(params);

    zt::ZeroTestKey zt_key;
    if (party.GetId() == 0) {
        key_io.ReadZeroTestKeyFromFile(kZtKeyPath_P0, params, zt_key);
    } else {
        key_io.ReadZeroTestKeyFromFile(kZtKeyPath_P1, params, zt_key);
    }

    // Start communication
    party.StartCommunication();

    uint32_t result{0}, xr_0{0}, xr_1{0};
    if (party.GetId() == 0) {
        xr_0 = utils::Mod(x + zt_key.shr_in, bitsize);
    } else {
        xr_1 = utils::Mod(x + zt_key.shr_in, bitsize);
    }
    uint32_t xr = ss.Reconst(party, xr_0, xr_1);

    result = zt.EvaluateAt(zt_key, xr);
    zt_key.FreeZeroTestKey();

    return result;
}

uint32_t Equality(tools::secret_sharing::Party &party, const uint32_t x, const uint32_t y, const uint32_t bitsize) {
    zt::ZeroTestParameters                       params(bitsize, bitsize, dbg_info);
    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);
    zt::ZeroTest                                 zt(params);

    zt::ZeroTestKey zt_key;
    if (party.GetId() == 0) {
        key_io.ReadZeroTestKeyFromFile(kEqKeyPath_P0, params, zt_key);
    } else {
        key_io.ReadZeroTestKeyFromFile(kEqKeyPath_P1, params, zt_key);
    }

    // Start communication
    party.StartCommunication();

    uint32_t result{0}, xr_0{0}, xr_1{0};
    if (party.GetId() == 0) {
        xr_0 = utils::Mod(x - y + zt_key.shr_in, bitsize);
    } else {
        xr_1 = utils::Mod(x - y + zt_key.shr_in, bitsize);
    }
    uint32_t xr = ss.Reconst(party, xr_0, xr_1);

    result = zt.EvaluateAt(zt_key, xr);
    zt_key.FreeZeroTestKey();

    return result;
}

uint32_t Compare(tools::secret_sharing::Party &party, const uint32_t x, const uint32_t y, const uint32_t bitsize) {
    comp::CompParameters                         params(bitsize, bitsize, dbg_info);
    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);
    comp::IntegerComparison                      comp(params);

    comp::CompKey comp_key;
    if (party.GetId() == 0) {
        key_io.ReadCompKeyFromFile(kCompKeyPath_P0, bitsize, comp_key);
    } else {
        key_io.ReadCompKeyFromFile(kCompKeyPath_P1, bitsize, comp_key);
    }

    // Start communication
    party.StartCommunication();

    uint32_t result{0}, xr_0{0}, xr_1{0}, yr_0{0}, yr_1{0};
    if (party.GetId() == 0) {
        xr_0 = utils::Mod(x + comp_key.shr1_in, bitsize);
        yr_0 = utils::Mod(y + comp_key.shr2_in, bitsize);
    } else {
        xr_1 = utils::Mod(x + comp_key.shr1_in, bitsize);
        yr_1 = utils::Mod(y + comp_key.shr2_in, bitsize);
    }
    uint32_t xr = ss.Reconst(party, xr_0, xr_1);
    uint32_t yr = ss.Reconst(party, yr_0, yr_1);

    if (party.GetId() == 0) {
        result = comp.Evaluate(comp_key, xr, yr) - comp_key.shr_out;
    } else {
        result = comp.Evaluate(comp_key, xr, yr) - comp_key.shr_out;
    }

    comp_key.FreeCompKey();

    return result;
}

std::vector<uint32_t> FMISearch(tools::secret_sharing::Party &party, const std::vector<uint32_t> &q, const uint32_t bitsize) {
    fmi::FssFmiParameters                        params(bitsize, kMaxQuerySize, dbg_info);
    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);
    tools::secret_sharing::ShareHandler          sh;
    utils::FileIo                                io;
    uint32_t                                     qs = q.size();
    fmi::FssFmi                                  fss_fmi(params);

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
    fmi::FssFmiKey fmi_key;
    if (party.GetId() == 0) {
        key_io.ReadFssFmiKeyFromFile(kFMIKeyPath_P0, params, fmi_key);
    } else {
        key_io.ReadFssFmiKeyFromFile(kFMIKeyPath_P1, params, fmi_key);
    }

    // Start communication
    party.StartCommunication();

    // Execute Eval^{FssFMI} algorithm
    std::vector<uint32_t> result(qs);
    if (party.GetId() == 0) {
        fss_fmi.Evaluate(party, fmi_key, q, result);
    } else {
        fss_fmi.Evaluate(party, fmi_key, q, result);
    }
    return result;
}

}    // namespace fss
