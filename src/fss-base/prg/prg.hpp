/**
 * @file prg.hpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-05-12
 * @copyright Copyright (c) 2024
 * @brief Pseudo-Random Generator (PRG) class.
 */

#ifndef PRG_PRG_H_
#define PRG_PRG_H_

#include <openssl/err.h>
#include <openssl/evp.h>

#include "../fss_block.hpp"
#include "../fss_configure.hpp"
#include "aes.hpp"

namespace fss {
namespace prg {

namespace details {

// Error codes for the PseudorandomGenerator class.
constexpr int ERR_EVP_CTX_NEW       = -1;    // Error creating EVP_CIPHER_CTX.
constexpr int ERR_EVP_CIPHER_INIT   = -2;    // Error initializing EVP_CIPHER.
constexpr int ERR_EVP_CIPHER_UPDATE = -3;    // Error updating EVP_CIPHER.

enum PRGType
{
    AES_NI, /**< AES-NI PRG. */
    OPENSSL /**< OpenSSL PRG. */
};

/**
 * @class PseudorandomGenerator
 * @brief Class representing a pseudorandom generator using AES-NI.
 */
template <PRGType types>
class PseudorandomGenerator {
public:
    // PseudorandomGenerator is not copyable.
    PseudorandomGenerator(const PseudorandomGenerator &)            = delete;
    PseudorandomGenerator &operator=(const PseudorandomGenerator &) = delete;

    // PseudorandomGenerator is movable.
    PseudorandomGenerator(PseudorandomGenerator &&)            = default;
    PseudorandomGenerator &operator=(PseudorandomGenerator &&) = default;

    /**
     * @brief Create a PseudorandomGenerator instance with the provided key.
     * @param key The key to initialize the pseudorandom generator.
     * @param debug Whether to enable debug utils::Mode for printing.
     * @return The created PseudorandomGenerator instance.
     */
    static PseudorandomGenerator Create(const Block &key, const bool debug = false);

    /**
     * @brief Evaluate the pseudorandom generator with the input seed and produce an output seed.
     * @param seed_in The input seed.
     * @param seed_out The output seed generated by the pseudorandom generator.
     */
    void Evaluate(const Block &seed_in, Block &seed_out, const bool debug = false) const;

    /**
     * @brief Evaluate the pseudorandom generator with the input seeds and produce output seeds.
     * @param seed_in The input seeds.
     * @param seed_out The output seeds generated by the pseudorandom generator.
     */
    void Evaluate(const std::array<Block, 8> &seed_in, std::array<Block, 8> &seed_out, const bool debug = false) const;

private:
    AES             aes_;     /**< The AES instance used for the pseudorandom generator. */
    EVP_CIPHER_CTX *prg_ctx_; /**< OpenSSL EVP cipher context. */

    /**
     * @brief Private constructor to create a PseudorandomGenerator instance with the provided key.
     * @param user_key The key to initialize the pseudorandom generator.
     */
    PseudorandomGenerator(const Block &user_key);

    /**
     * @brief Private constructor to create a PseudorandomGenerator instance with an EVP_CIPHER_CTX.
     * @param prg_ctx The OpenSSL EVP cipher context.
     * @param debug Whether to enable debug utils::Mode for printing.
     */
    PseudorandomGenerator(EVP_CIPHER_CTX *prg_ctx);
};

}    // namespace details

#ifdef AES_NI_ENABLED
using PRG = details::PseudorandomGenerator<details::AES_NI>;
#else
using PRG = details::PseudorandomGenerator<details::OPENSSL>;
#endif

namespace test {

void Test_Prg(TestInfo &test_info);

}    // namespace test

}    // namespace prg
}    // namespace fss

#endif    // PRG_PRG_H_
