/**
 * @file fssgate.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-12-11
 * @copyright Copyright (c) 2024
 * @brief
 */

#ifndef FSSGATE_H_
#define FSSGATE_H_

#include <vector>

#include "../fss-gate/zt/zero_test_dpf.hpp"
#include "../tools/secret_sharing.hpp"

namespace fss {

void ZeroTestSetup(const uint32_t bitsize = 32);
void EqualitySetup(const uint32_t bitsize = 32);
void CompareSetup(const uint32_t bitsize = 32);                                  // * 1 is x<y else 0 (ただし|x-y| < 2^(n-1)しか判定できない)
void FMISearchSetup(const uint32_t bitsize, std::vector<uint32_t> &database);    // * MaxQuerySize is 2^7 = 128

uint32_t              ZeroTest(tools::secret_sharing::Party &party, const uint32_t x, const uint32_t bitsize = 32);
uint32_t              Equality(tools::secret_sharing::Party &party, const uint32_t x, const uint32_t y, const uint32_t bitsize = 32);
uint32_t              Compare(tools::secret_sharing::Party &party, const uint32_t x, const uint32_t y, const uint32_t bitsize = 32);
std::vector<uint32_t> FMISearch(tools::secret_sharing::Party &party, const std::vector<uint32_t> &q, const uint32_t bitsize = 32);

}    // namespace fss

#endif    // FSSGATE_H_
