/**
 * @file tools.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Tools class.
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include "../comm/comm.hpp"

namespace tools {

namespace test {

void Test_SecretSharing(const comm::CommInfo &comm_info, const uint32_t mode, bool debug);

}    // namespace test

}    // namespace tools

#endif    // TOOLS_H_
