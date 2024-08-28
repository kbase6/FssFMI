/**
 * @file aes.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-05-12
 * @copyright Copyright (c) 2024
 * @brief AES class.
 */

#ifndef PRG_AES_H_
#define PRG_AES_H_

#include <wmmintrin.h>

#include "../fss_block.hpp"

namespace fss {
namespace prg {

class AES {
public:
    // Default constructor leave the class in an invalid state
    // until setKey(...) is called.
    AES()            = default;
    AES(const AES &) = default;

    // Constructor to initialize the class with the given key
    AES(const Block &user_key);

    // Set the key to be used for encryption.
    void SetKey(const Block &user_key);

    // Encrypts the plaintext block and stores the result in ciphertext
    void EcbEncBlock(const Block &plaintext, Block &ciphertext) const;

    // Encrypts the plaintext block and returns the result
    Block EcbEncBlock(const Block &plaintext) const;

    void EcbEncBlocks(const std::array<Block, 8> &plaintexts, std::array<Block, 8> &ciphertext) const;

    static Block RoundEnc(Block state, const Block &roundKey);
    static Block FinalEnc(Block state, const Block &roundKey);

    // The expanded key.
    std::array<Block, 11> round_key;
};

inline Block AES::RoundEnc(Block state, const Block &roundKey) {
    return _mm_aesenc_si128(state, roundKey);
}

inline Block AES::FinalEnc(Block state, const Block &roundKey) {
    return _mm_aesenclast_si128(state, roundKey);
}

class AESDec {
public:
    AESDec()               = default;
    AESDec(const AESDec &) = default;
    AESDec(const Block &user_key);

    void  SetKey(const Block &userKey);
    void  EcbDecBlock(const Block &ciphertext, Block &plaintext);
    Block EcbDecBlock(const Block &ciphertext);

    static Block RoundDec(Block state, const Block &roundKey);
    static Block FinalDec(Block state, const Block &roundKey);

    std::array<Block, 11> round_key;
};

}    // namespace prg
}    // namespace fss

#endif    // PRG_AES_H_
