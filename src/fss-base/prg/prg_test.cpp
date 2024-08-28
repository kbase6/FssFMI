/**
 * @file prg_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief PRG test implementation.
 */

#include "prg.hpp"

#include "../../utils/logger.hpp"
#include "../../utils/timer.hpp"
#include "../../utils/utils.hpp"

namespace fss {
namespace prg {
namespace test {

bool Test_SetKey(const TestInfo &test_info);
bool Test_EcbEncBlock(const TestInfo &test_info);
bool Test_EcbEncBlock_Return(const TestInfo &test_info);
bool Test_EcbEncBlocks(const TestInfo &test_info);
bool Test_EcbDecBlock(const TestInfo &test_info);
bool Test_EcbDecBlock_Return(const TestInfo &test_info);
bool Test_Prg_AESNI_Evaluate(const TestInfo &test_info);
bool Test_Prg_AESNI_Evaluate_Multiple(const TestInfo &test_info);
bool Test_Prg_OpenSSL_Evaluate(const TestInfo &test_info);
bool Test_Prg_OpenSSL_Evaluate_Multiple(const TestInfo &test_info);

void Test_Prg(TestInfo &test_info) {
    std::vector<std::string> modes         = {"PRG unit tests", "AESEncryption", "AESDecryption", "PRG_AESNI", "PRG_OpenSSL"};
    uint32_t                 selected_mode = test_info.mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    utils::PrintText(utils::Logger::StrWithSep(modes[selected_mode - 1]));
    if (selected_mode == 1) {
        test_info.dbg_info.debug = false;
        utils::PrintTestResult("Test_SetKey", Test_SetKey(test_info));
        utils::PrintTestResult("Test_EcbEncBlock", Test_EcbEncBlock(test_info));
        utils::PrintTestResult("Test_EcbEncBlock_Return", Test_EcbEncBlock_Return(test_info));
        utils::PrintTestResult("Test_EcbEncBlocks", Test_EcbEncBlocks(test_info));
        utils::PrintTestResult("Test_EcbDecBlock", Test_EcbDecBlock(test_info));
        utils::PrintTestResult("Test_EcbDecBlock_Return", Test_EcbDecBlock_Return(test_info));
        utils::PrintTestResult("Test_Prg_AESNI_Evaluate", Test_Prg_AESNI_Evaluate(test_info));
        utils::PrintTestResult("Test_Prg_AESNI_Evaluate_Multiple", Test_Prg_AESNI_Evaluate_Multiple(test_info));
        utils::PrintTestResult("Test_Prg_OpenSSL_Evaluate", Test_Prg_OpenSSL_Evaluate(test_info));
        utils::PrintTestResult("Test_Prg_OpenSSL_Evaluate_Multiple", Test_Prg_OpenSSL_Evaluate_Multiple(test_info));
    } else if (selected_mode == 2) {
        utils::PrintTestResult("Test_EcbEncBlock", Test_EcbEncBlock(test_info));
        utils::PrintTestResult("Test_EcbEncBlock_Return", Test_EcbEncBlock_Return(test_info));
        utils::PrintTestResult("Test_EcbEncBlocks", Test_EcbEncBlocks(test_info));
    } else if (selected_mode == 3) {
        utils::PrintTestResult("Test_EcbDecBlock", Test_EcbDecBlock(test_info));
        utils::PrintTestResult("Test_EcbDecBlock_Return", Test_EcbDecBlock_Return(test_info));
    } else if (selected_mode == 4) {
        utils::PrintTestResult("Test_Prg_AESNI_Evaluate", Test_Prg_AESNI_Evaluate(test_info));
        utils::PrintTestResult("Test_Prg_AESNI_Evaluate_Multiple", Test_Prg_AESNI_Evaluate_Multiple(test_info));
    } else if (selected_mode == 5) {
        utils::PrintTestResult("Test_Prg_OpenSSL_Evaluate", Test_Prg_OpenSSL_Evaluate(test_info));
        utils::PrintTestResult("Test_Prg_OpenSSL_Evaluate_Multiple", Test_Prg_OpenSSL_Evaluate_Multiple(test_info));
    }
    utils::PrintText(utils::kDash);
}

const Block kPrgKeyTest{0x304a17ca6c3e0e01, 0x50a32153426e6367};    // First half of the SHA256 hash for `fss::rng::kPrgKeyTest`

bool Test_SetKey(const TestInfo &test_info) {
    AES aes;
    aes.SetKey(kPrgKeyTest);
    // Check if the key is set correctly
    return aes.round_key[0] == kPrgKeyTest;
}

bool Test_EcbEncBlock(const TestInfo &test_info) {
    AES   aes(kPrgKeyTest);
    Block plaintext{0x123456789abcdef0, 0xfedcba9876543210};
    Block ciphertext;
    aes.EcbEncBlock(plaintext, ciphertext);
    plaintext.PrintBlockHexDebug(LOCATION, "plaintext: ", test_info.dbg_info.debug);
    ciphertext.PrintBlockHexDebug(LOCATION, "ciphertext: ", test_info.dbg_info.debug);

    // Check if encryption produces non-zero ciphertext
    return ciphertext != plaintext && ciphertext != Block{0, 0};
}

bool Test_EcbEncBlock_Return(const TestInfo &test_info) {
    AES   aes(kPrgKeyTest);
    Block plaintext{0x123456789abcdef0, 0xfedcba9876543210};
    Block ciphertext = aes.EcbEncBlock(plaintext);
    plaintext.PrintBlockHexDebug(LOCATION, "plaintext: ", test_info.dbg_info.debug);
    ciphertext.PrintBlockHexDebug(LOCATION, "ciphertext: ", test_info.dbg_info.debug);

    // Check if encryption produces non-zero ciphertext
    return ciphertext != plaintext && ciphertext != Block{0, 0};
}

bool Test_EcbEncBlocks(const TestInfo &test_info) {
    AES                  aes(kPrgKeyTest);
    std::array<Block, 8> plaintexts = {
        Block{0x123456789abcdef0, 0xfedcba9876543210},
        Block{0x23456789abcdef01, 0xedcba9876543210f},
        Block{0x3456789abcdef012, 0xdcba9876543210fe},
        Block{0x456789abcdef0123, 0xcba9876543210fed},
        Block{0x56789abcdef01234, 0xba9876543210fedc},
        Block{0x6789abcdef012345, 0xa9876543210fedcb},
        Block{0x789abcdef0123456, 0x9876543210fedcba},
        Block{0x89abcdef01234567, 0x876543210fedcbaf}};
    std::array<Block, 8> ciphertexts;
    aes.EcbEncBlocks(plaintexts, ciphertexts);

    // Check if encryption produces non-zero ciphertexts
    bool all_non_zero = true;
    for (int i = 0; i < 8; ++i) {
        plaintexts[i].PrintBlockHexDebug(LOCATION, "plaintexts[" + std::to_string(i) + "]: ", test_info.dbg_info.debug);
        ciphertexts[i].PrintBlockHexDebug(LOCATION, "ciphertexts[" + std::to_string(i) + "]: ", test_info.dbg_info.debug);
        if (ciphertexts[i] == plaintexts[i] || ciphertexts[i] == Block{0, 0}) {
            all_non_zero = false;
            break;
        }
    }
    return all_non_zero;
}

bool Test_EcbDecBlock(const TestInfo &test_info) {
    AES    aes(kPrgKeyTest);
    AESDec aes_dec(kPrgKeyTest);
    Block  plaintext{0x123456789abcdef0, 0xfedcba9876543210};
    Block  ciphertext;
    Block  decrypted;

    plaintext.PrintBlockHexDebug(LOCATION, "plaintext: ", test_info.dbg_info.debug);
    aes.EcbEncBlock(plaintext, ciphertext);
    ciphertext.PrintBlockHexDebug(LOCATION, "ciphertext: ", test_info.dbg_info.debug);
    aes_dec.EcbDecBlock(ciphertext, decrypted);
    decrypted.PrintBlockHexDebug(LOCATION, "decrypted: ", test_info.dbg_info.debug);

    // Check if decrypted text matches original plaintext
    return decrypted == plaintext;
}

bool Test_EcbDecBlock_Return(const TestInfo &test_info) {
    AES    aes(kPrgKeyTest);
    AESDec aes_dec(kPrgKeyTest);
    Block  plaintext{0x123456789abcdef0, 0xfedcba9876543210};
    Block  ciphertext;
    Block  decrypted;

    plaintext.PrintBlockHexDebug(LOCATION, "plaintext: ", test_info.dbg_info.debug);
    aes.EcbEncBlock(plaintext, ciphertext);
    ciphertext.PrintBlockHexDebug(LOCATION, "ciphertext: ", test_info.dbg_info.debug);
    decrypted = aes_dec.EcbDecBlock(ciphertext);
    decrypted.PrintBlockHexDebug(LOCATION, "decrypted: ", test_info.dbg_info.debug);

    // Check if decrypted text matches original plaintext
    return decrypted == plaintext;
}

bool Test_Prg_AESNI_Evaluate(const TestInfo &test_info) {
    details::PseudorandomGenerator prg = details::PseudorandomGenerator<details::AES_NI>::Create(kPrgKeyTest);
    fss::Block                     seed_in{0x123456789abcdef0, 0xfedcba9876543210};
    fss::Block                     seed_out;
    prg.Evaluate(seed_in, seed_out);
    seed_in.PrintBlockHexDebug(LOCATION, "seed_in: ", test_info.dbg_info.debug);
    seed_out.PrintBlockHexDebug(LOCATION, "seed_out: ", test_info.dbg_info.debug);

    // Check if evaluation produces non-zero output seed
    return seed_out != seed_in && seed_out != fss::Block{0, 0};
}

bool Test_Prg_AESNI_Evaluate_Multiple(const TestInfo &test_info) {
    details::PseudorandomGenerator prg     = details::PseudorandomGenerator<details::AES_NI>::Create(kPrgKeyTest);
    std::array<fss::Block, 8>      seed_in = {
        fss::Block{0x123456789abcdef0, 0xfedcba9876543210},
        fss::Block{0x23456789abcdef01, 0xedcba9876543210f},
        fss::Block{0x3456789abcdef012, 0xdcba9876543210fe},
        fss::Block{0x456789abcdef0123, 0xcba9876543210fed},
        fss::Block{0x56789abcdef01234, 0xba9876543210fedc},
        fss::Block{0x6789abcdef012345, 0xa9876543210fedcb},
        fss::Block{0x789abcdef0123456, 0x9876543210fedcba},
        fss::Block{0x89abcdef01234567, 0x876543210fedcbaf}};
    std::array<fss::Block, 8> seed_out;
    prg.Evaluate(seed_in, seed_out);

    // Check if evaluation produces non-zero output seeds
    bool all_non_zero = true;
    for (int i = 0; i < 8; ++i) {
        seed_in[i].PrintBlockHexDebug(LOCATION, "seed_in[" + std::to_string(i) + "]: ", test_info.dbg_info.debug);
        seed_out[i].PrintBlockHexDebug(LOCATION, "seed_out[" + std::to_string(i) + "]: ", test_info.dbg_info.debug);
        if (seed_out[i] == seed_in[i] || seed_out[i] == fss::Block{0, 0}) {
            all_non_zero = false;
            break;
        }
    }
    return all_non_zero;
}

bool Test_Prg_OpenSSL_Evaluate(const TestInfo &test_info) {
    details::PseudorandomGenerator prg = details::PseudorandomGenerator<details::OPENSSL>::Create(kPrgKeyTest);
    fss::Block                     seed_in{0x123456789abcdef0, 0xfedcba9876543210};
    fss::Block                     seed_out;
    prg.Evaluate(seed_in, seed_out);
    seed_in.PrintBlockHexDebug(LOCATION, "seed_in: ", test_info.dbg_info.debug);
    seed_out.PrintBlockHexDebug(LOCATION, "seed_out: ", test_info.dbg_info.debug);

    // Check if evaluation produces non-zero output seed
    return seed_out != seed_in && seed_out != fss::Block{0, 0};
}

bool Test_Prg_OpenSSL_Evaluate_Multiple(const TestInfo &test_info) {
    details::PseudorandomGenerator prg     = details::PseudorandomGenerator<details::OPENSSL>::Create(kPrgKeyTest);
    std::array<fss::Block, 8>      seed_in = {
        fss::Block{0x123456789abcdef0, 0xfedcba9876543210},
        fss::Block{0x23456789abcdef01, 0xedcba9876543210f},
        fss::Block{0x3456789abcdef012, 0xdcba9876543210fe},
        fss::Block{0x456789abcdef0123, 0xcba9876543210fed},
        fss::Block{0x56789abcdef01234, 0xba9876543210fedc},
        fss::Block{0x6789abcdef012345, 0xa9876543210fedcb},
        fss::Block{0x789abcdef0123456, 0x9876543210fedcba},
        fss::Block{0x89abcdef01234567, 0x876543210fedcbaf}};
    std::array<fss::Block, 8> seed_out;
    prg.Evaluate(seed_in, seed_out);

    // Check if evaluation produces non-zero output seeds
    bool all_non_zero = true;
    for (int i = 0; i < 8; ++i) {
        seed_in[i].PrintBlockHexDebug(LOCATION, "seed_in[" + std::to_string(i) + "]: ", test_info.dbg_info.debug);
        seed_out[i].PrintBlockHexDebug(LOCATION, "seed_out[" + std::to_string(i) + "]: ", test_info.dbg_info.debug);
        if (seed_out[i] == seed_in[i] || seed_out[i] == fss::Block{0, 0}) {
            all_non_zero = false;
            break;
        }
    }
    return all_non_zero;
}

}    // namespace test
}    // namespace prg
}    // namespace fss
