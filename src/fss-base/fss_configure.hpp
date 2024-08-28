/**
 * @file fss_configure.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-10
 * @copyright Copyright (c) 2024
 * @brief FSS configure class.
 */

#ifndef FSS_FSS_CONFIGURE_H_
#define FSS_FSS_CONFIGURE_H_

#include <vector>

#include "fss_block.hpp"

namespace fss {

constexpr uint32_t kSecurityParameter = 128;    // The security parameter in bits.

const Block kPrgKeySeedLeft{0xf2416bf54f02e446, 0xcc2ce93fdbcccc28};      // First half of the SHA256 hash for `dpf::kPrgKeySeedLeft`
const Block kPrgKeySeedRight{0x65776b0991b8d225, 0xdac18583c2123349};     // First half of the SHA256 hash for `dpf::kPrgKeySeedRight`
const Block kPrgKeyValueLeft{0x276bea362956385d, 0xd496c0250718166b};     // First half of the SHA256 hash for `dpf::kPrgKeyValueLeft`
const Block kPrgKeyValueRight{0xaaa420b202808958, 0xf426148f7bfb9254};    // First half of the SHA256 hash for `dpf::kPrgKeyValueRight`

struct DebugInfo {
    bool dpf_debug{false};
    bool dcf_debug{false};
    bool ddcf_debug{false};
    bool comp_debug{false};
    bool zt_debug{false};
    bool select_debug{false};
    bool rank_debug{false};
    bool fmi_debug{false};
    bool debug{false};

    DebugInfo(){};
};

struct TestInfo {
    uint32_t              mode = 0;
    DebugInfo             dbg_info;
    std::vector<uint32_t> domain_size;

    TestInfo(){};
};

struct BenchInfo {
    uint32_t              experiment_num = 3;
    uint32_t              mode           = 0;
    uint32_t              limit_time_ms  = 7200000;    // 2 hours
    std::vector<uint32_t> text_size;
    std::vector<uint32_t> query_size;
    DebugInfo             dbg_info;

    BenchInfo(){};
};

}    // namespace fss

#endif    // FSS_FSS_CONFIGURE_H_
