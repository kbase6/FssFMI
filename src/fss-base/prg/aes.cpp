/**
 * @file aes.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-05-12
 * @copyright Copyright (c) 2024
 * @brief AES implementation.
 */

#include "aes.hpp"
#include <cstring>
#include <iostream>

namespace fss {
namespace prg {

Block keyGenHelper(Block key, Block key_rcon) {
    key_rcon = _mm_shuffle_epi32(key_rcon, _MM_SHUFFLE(3, 3, 3, 3));
    key      = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key      = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key      = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    return _mm_xor_si128(key, key_rcon);
};

void AES::SetKey(const Block &user_key) {
    round_key[0]  = user_key;
    round_key[1]  = keyGenHelper(round_key[0], _mm_aeskeygenassist_si128(round_key[0], 0x01));
    round_key[2]  = keyGenHelper(round_key[1], _mm_aeskeygenassist_si128(round_key[1], 0x02));
    round_key[3]  = keyGenHelper(round_key[2], _mm_aeskeygenassist_si128(round_key[2], 0x04));
    round_key[4]  = keyGenHelper(round_key[3], _mm_aeskeygenassist_si128(round_key[3], 0x08));
    round_key[5]  = keyGenHelper(round_key[4], _mm_aeskeygenassist_si128(round_key[4], 0x10));
    round_key[6]  = keyGenHelper(round_key[5], _mm_aeskeygenassist_si128(round_key[5], 0x20));
    round_key[7]  = keyGenHelper(round_key[6], _mm_aeskeygenassist_si128(round_key[6], 0x40));
    round_key[8]  = keyGenHelper(round_key[7], _mm_aeskeygenassist_si128(round_key[7], 0x80));
    round_key[9]  = keyGenHelper(round_key[8], _mm_aeskeygenassist_si128(round_key[8], 0x1B));
    round_key[10] = keyGenHelper(round_key[9], _mm_aeskeygenassist_si128(round_key[9], 0x36));
}

AES::AES(const Block &user_key) {
    SetKey(user_key);
}

void AES::EcbEncBlock(const Block &plaintext, Block &ciphertext) const {
    ciphertext = plaintext ^ round_key[0];
    for (uint32_t i = 1; i < 10; i++) {
        ciphertext = RoundEnc(ciphertext, round_key[i]);
    }
    ciphertext = FinalEnc(ciphertext, round_key[10]);
}

Block AES::EcbEncBlock(const Block &plaintext) const {
    Block ret;
    EcbEncBlock(plaintext, ret);
    return ret;
}

void AES::EcbEncBlocks(const std::array<Block, 8> &plaintexts, std::array<Block, 8> &ciphertext) const {
    for (uint32_t i = 0; i < 8; i++) {
        EcbEncBlock(plaintexts[i], ciphertext[i]);
    }
}

AESDec::AESDec(const Block &user_key) {
    SetKey(user_key);
}

void AESDec::SetKey(const Block &user_key) {
    const Block &v0  = user_key;
    const Block  v1  = keyGenHelper(v0, _mm_aeskeygenassist_si128(v0, 0x01));
    const Block  v2  = keyGenHelper(v1, _mm_aeskeygenassist_si128(v1, 0x02));
    const Block  v3  = keyGenHelper(v2, _mm_aeskeygenassist_si128(v2, 0x04));
    const Block  v4  = keyGenHelper(v3, _mm_aeskeygenassist_si128(v3, 0x08));
    const Block  v5  = keyGenHelper(v4, _mm_aeskeygenassist_si128(v4, 0x10));
    const Block  v6  = keyGenHelper(v5, _mm_aeskeygenassist_si128(v5, 0x20));
    const Block  v7  = keyGenHelper(v6, _mm_aeskeygenassist_si128(v6, 0x40));
    const Block  v8  = keyGenHelper(v7, _mm_aeskeygenassist_si128(v7, 0x80));
    const Block  v9  = keyGenHelper(v8, _mm_aeskeygenassist_si128(v8, 0x1B));
    const Block  v10 = keyGenHelper(v9, _mm_aeskeygenassist_si128(v9, 0x36));

    _mm_storeu_si128(&round_key[0].m128i(), v10);
    _mm_storeu_si128(&round_key[1].m128i(), _mm_aesimc_si128(v9));
    _mm_storeu_si128(&round_key[2].m128i(), _mm_aesimc_si128(v8));
    _mm_storeu_si128(&round_key[3].m128i(), _mm_aesimc_si128(v7));
    _mm_storeu_si128(&round_key[4].m128i(), _mm_aesimc_si128(v6));
    _mm_storeu_si128(&round_key[5].m128i(), _mm_aesimc_si128(v5));
    _mm_storeu_si128(&round_key[6].m128i(), _mm_aesimc_si128(v4));
    _mm_storeu_si128(&round_key[7].m128i(), _mm_aesimc_si128(v3));
    _mm_storeu_si128(&round_key[8].m128i(), _mm_aesimc_si128(v2));
    _mm_storeu_si128(&round_key[9].m128i(), _mm_aesimc_si128(v1));
    _mm_storeu_si128(&round_key[10].m128i(), v0);
}

Block AESDec::RoundDec(Block state, const Block &roundKey) {
    return _mm_aesdec_si128(state, roundKey);
}

Block AESDec::FinalDec(Block state, const Block &roundKey) {
    return _mm_aesdeclast_si128(state, roundKey);
}

void AESDec::EcbDecBlock(const Block &ciphertext, Block &plaintext) {

    plaintext = ciphertext ^ round_key[0];
    plaintext = RoundDec(plaintext, round_key[1]);
    plaintext = RoundDec(plaintext, round_key[2]);
    plaintext = RoundDec(plaintext, round_key[3]);
    plaintext = RoundDec(plaintext, round_key[4]);
    plaintext = RoundDec(plaintext, round_key[5]);
    plaintext = RoundDec(plaintext, round_key[6]);
    plaintext = RoundDec(plaintext, round_key[7]);
    plaintext = RoundDec(plaintext, round_key[8]);
    plaintext = RoundDec(plaintext, round_key[9]);
    plaintext = FinalDec(plaintext, round_key[10]);
}

Block AESDec::EcbDecBlock(const Block &plaintext) {
    Block ret;
    EcbDecBlock(plaintext, ret);
    return ret;
}

}    // namespace prg
}    // namespace fss
