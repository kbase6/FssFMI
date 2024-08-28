/**
 * @file prg.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Pseudo-Random Generator (PRG) implementation.
 */

#include "prg.hpp"

#include "../../utils/logger.hpp"
#include "../../utils/utils.hpp"

namespace fss {
namespace prg {

namespace details {

template <>
PseudorandomGenerator<AES_NI>::PseudorandomGenerator(const Block &user_key)
    : aes_(AES(user_key)) {
}

template <>
PseudorandomGenerator<AES_NI> PseudorandomGenerator<AES_NI>::Create(const Block &key, const bool debug) {
    // key.PrintBlockHex(LOCATION, "PRG key: ", debug);   // DEBUG_OFF
    return PseudorandomGenerator(key);
}

template <>
void PseudorandomGenerator<AES_NI>::Evaluate(const Block &seed_in, Block &seed_out, const bool debug) const {
    // seed_in.PrintBlockHex(LOCATION, "PRG input seed: ", debug);   // DEBUG_OFF
    aes_.EcbEncBlock(seed_in, seed_out);
    // seed_out.PrintBlockHex(LOCATION, "PRG output seed: ", debug);   // DEBUG_OFF
}

template <>
void PseudorandomGenerator<AES_NI>::Evaluate(const std::array<Block, 8> &seed_in, std::array<Block, 8> &seed_out, const bool debug) const {
    aes_.EcbEncBlocks(seed_in, seed_out);
}

template <>
PseudorandomGenerator<OPENSSL>::PseudorandomGenerator(EVP_CIPHER_CTX *prg_ctx)
    : prg_ctx_(std::move(prg_ctx)) {
}

template <>
PseudorandomGenerator<OPENSSL> PseudorandomGenerator<OPENSSL>::Create(const Block &key, const bool debug) {
    // utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Setup PRG key"), debug);    // DEBUG OFF
    // Setup PRG(AES) key.
    EVP_CIPHER_CTX *prg_ctx = EVP_CIPHER_CTX_new();
    if (!prg_ctx) {
        utils::Logger::FatalLog(LOCATION, "Failed to allocate AES context");
        exit(ERR_EVP_CTX_NEW);
    }
    // Convert to `unsigned char` from `Block`.
    unsigned char prg_key[16];
    _mm_storeu_si128((__m128i *)prg_key, key);
    // utils::Logger::TraceLog(LOCATION, "Allocate AES context", debug);    // DEBUG OFF
    int openssl_status = EVP_EncryptInit_ex(prg_ctx, EVP_aes_128_ecb(), nullptr, prg_key, nullptr);
    if (openssl_status != 1) {
        utils::Logger::FatalLog(LOCATION, "Failed to setup AES context");
        exit(ERR_EVP_CIPHER_INIT);
    }
    // utils::Logger::TraceLog(LOCATION, "Setup AES context", debug);    // DEBUG OFF
    return PseudorandomGenerator<OPENSSL>(prg_ctx);
}

template <>
void PseudorandomGenerator<OPENSSL>::Evaluate(const Block &seed_in, Block &seed_out, const bool debug) const {
    // utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate PRG"), debug);    // DEBUG OFF
    int           output_length;
    unsigned char prg_output[16];
    unsigned char prg_input[16];
    // Apply sigma function.
    Block sigma_in(seed_in.GetHigh(), seed_in.GetHigh() ^ seed_in.GetLow());
    // Convert to `unsigned char` from `array<byte, 16>`.
    _mm_storeu_si128((__m128i *)prg_input, seed_in);
    // Compute PRG output (encryption).
    int openssl_status = EVP_EncryptUpdate(this->prg_ctx_, prg_output, &output_length, prg_input, 16);
    if (openssl_status != 1) {
        utils::Logger::FatalLog(LOCATION, "AES encryption failed");
        exit(ERR_EVP_CIPHER_UPDATE);
    }
    // utils::Logger::TraceLog(LOCATION, "Finish AES encryption", debug);    // DEBUG OFF
    // Convert to `Block` from `unsigned char`.
    seed_out = Block(_mm_loadu_si128((__m128i *)prg_output));
    // seed_out.PrintBlockHex(LOCATION, "PRG output: ", debug);    // DEBUG OFF
}

template <>
void PseudorandomGenerator<OPENSSL>::Evaluate(const std::array<Block, 8> &seed_in, std::array<Block, 8> &seed_out, const bool debug) const {
    // utils::Logger::TraceLog(LOCATION, utils::Logger::StrWithSep("Evaluate PRG"), debug);    // DEBUG OFF
    unsigned char prg_input[16];
    int           output_length;
    unsigned char prg_output[16];
    for (int i = 0; i < 8; i++) {
        // Apply sigma function.
        Block sigma_in(seed_in[i].GetHigh(), seed_in[i].GetHigh() ^ seed_in[i].GetLow());
        // Convert to `unsigned char` from `array<byte, 16>`.
        _mm_storeu_si128((__m128i *)prg_input, seed_in[i]);
        // Compute PRG output (encryption).
        int openssl_status = EVP_EncryptUpdate(this->prg_ctx_, prg_output, &output_length, prg_input, 16);
        if (openssl_status != 1) {
            utils::Logger::FatalLog(LOCATION, "AES encryption failed");
            exit(ERR_EVP_CIPHER_UPDATE);
        }
        // utils::Logger::TraceLog(LOCATION, "Finish AES encryption", debug);    // DEBUG OFF
        // Convert to `Block` from `unsigned char`.
        seed_out[i] = Block(_mm_loadu_si128((__m128i *)prg_output));
        // seed_out[i].PrintBlockHex(LOCATION, "PRG output: ", debug);    // DEBUG OFF
    }
}

}    // namespace details
}    // namespace prg
}    // namespace fss
