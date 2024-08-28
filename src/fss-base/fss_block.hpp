/**
 * @file fss_block.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief FSS block class.
 */

#ifndef FSS_FSS_BLOCK_H_
#define FSS_FSS_BLOCK_H_

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <emmintrin.h>
#include <smmintrin.h>
#include <wmmintrin.h>

namespace fss {

constexpr uint8_t kLeft  = 0;
constexpr uint8_t kRight = 1;
struct alignas(16) Block {
    __m128i data;

    Block()              = default;
    Block(const Block &) = default;
    Block(uint64_t high, uint64_t low) {
        data = _mm_set_epi64x(high, low);
    }
    Block(uint32_t e3, uint32_t e2, uint32_t e1, uint32_t e0) {
        data = _mm_set_epi32(e3, e2, e1, e0);
    }
    Block(uint16_t e7, uint16_t e6, uint16_t e5, uint16_t e4, uint16_t e3, uint16_t e2, uint16_t e1, uint16_t e0) {
        data = _mm_set_epi16(e7, e6, e5, e4, e3, e2, e1, e0);
    }
    Block(uint8_t e15, uint8_t e14, uint8_t e13, uint8_t e12, uint8_t e11, uint8_t e10, uint8_t e9, uint8_t e8,
          uint8_t e7, uint8_t e6, uint8_t e5, uint8_t e4, uint8_t e3, uint8_t e2, uint8_t e1, uint8_t e0) {
        data = _mm_set_epi8(e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0);
    }

    Block(const __m128i &x) {
        data = x;
    }

    operator const __m128i &() const {
        return data;
    }
    operator __m128i &() {
        return data;
    }

    __m128i &m128i() {
        return data;
    }
    const __m128i &m128i() const {
        return data;
    }

    template <typename T>
    typename std::enable_if<
        std::is_standard_layout<T>::value &&
            std::is_trivial<T>::value &&
            (sizeof(T) <= 16) &&
            (16 % sizeof(T) == 0),
        std::array<T, 16 / sizeof(T)> &>::type
    as() {
        return *(std::array<T, 16 / sizeof(T)> *)this;
    }

    template <typename T>
    typename std::enable_if<
        std::is_standard_layout<T>::value &&
            std::is_trivial<T>::value &&
            (sizeof(T) <= 16) &&
            (16 % sizeof(T) == 0),
        const std::array<T, 16 / sizeof(T)> &>::type
    as() const {
        return *(const std::array<T, 16 / sizeof(T)> *)this;
    }

    inline Block operator^(const Block &rhs) const {
        return mm_xor_si128(rhs);
    }

    inline Block mm_xor_si128(const Block &rhs) const {
        return _mm_xor_si128(*this, rhs);
    }

    inline Block operator&(const Block &rhs) const {
        return mm_and_si128(rhs);
    }

    inline Block mm_and_si128(const Block &rhs) const {
        return _mm_and_si128(*this, rhs);
    }

    inline Block operator|(const Block &rhs) const {
        return mm_or_si128(rhs);
    }
    inline Block mm_or_si128(const Block &rhs) const {
        return _mm_or_si128(*this, rhs);
    }

    inline Block operator<<(const std::uint8_t &rhs) const {
        return mm_slli_epi64(rhs);
    }

    inline Block mm_slli_epi64(const std::uint8_t &rhs) const {
        return _mm_slli_epi64(*this, rhs);
    }

    inline Block operator>>(const std::uint8_t &rhs) const {
        return mm_srli_epi64(rhs);
    }

    inline Block mm_srli_epi64(const std::uint8_t &rhs) const {
        return _mm_srli_epi64(*this, rhs);
    }

    inline Block operator+(const Block &rhs) const {
        return mm_add_epi64(rhs);
    }

    inline Block mm_add_epi64(const Block &rhs) const {
        return _mm_add_epi64(*this, rhs);
    }

    inline Block operator-(const Block &rhs) const {
        return mm_sub_epi64(rhs);
    }

    inline Block mm_sub_epi64(const Block &rhs) const {
        return _mm_sub_epi64(*this, rhs);
    }

    inline bool operator==(const Block &rhs) const {
        auto neq = _mm_xor_si128(*this, rhs);
        return _mm_test_all_zeros(neq, neq) != 0;
    }

    inline bool operator!=(const Block &rhs) const {
        return !(*this == rhs);
    }

    inline uint64_t GetHigh() const {
        return _mm_cvtsi128_si64(_mm_srli_si128(data, 8));
    }

    inline uint64_t GetLow() const {
        return _mm_cvtsi128_si64(data);
    }

    inline Block SetBit(const uint32_t n, const bool x) const {
        if (x) {
            __m128i mask = _mm_set_epi64x((n >= 64) ? 1UL << (n - 64) : 0, (n < 64) ? 1UL << n : 0);
            return _mm_or_si128(*this, mask);
        } else {
            return *this;
        }
    }

    void SetRandom();

    uint32_t Convert(const uint32_t bitsize) const;

    std::vector<uint32_t> ConvertVec(const uint32_t num, const uint32_t bit_size) const;

    void FromVec(const std::vector<uint32_t> &vec, const uint32_t num, const uint32_t bit_size);

    void PrintBlockHexTrace(const std::string &location, const std::string &msg, const bool debug) const;

    void PrintBlockBinTrace(const std::string &location, const std::string &msg, const bool debug) const;

    void PrintBlockHexDebug(const std::string &location, const std::string &msg, const bool debug) const;

    void PrintBlockBinDebug(const std::string &location, const std::string &msg, const bool debug) const;
};

inline Block ToBlock(std::uint64_t high_u64, std::uint64_t low_u64) {
    Block ret;
    ret.as<std::uint64_t>()[0] = low_u64;
    ret.as<std::uint64_t>()[1] = high_u64;
    return ret;
}

inline Block ToBlock(std::uint64_t low_u64) {
    return ToBlock(0, low_u64);
}

inline bool Lsb(const Block &b) {
    return _mm_cvtsi128_si64(b) & 1;
}

extern const Block                zero_block;
extern const Block                one_block;
extern const Block                all_one_block;
extern const std::array<Block, 2> zero_and_all_one;

}    // namespace fss

#endif    // FSS_FSS_BLOCK_H_
