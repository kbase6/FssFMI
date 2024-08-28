/**
 * @file fss_block.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief FSS block implementation.
 */

#include "fss_block.hpp"

#include <algorithm>
#include <iomanip>

#include "../tools/random_number_generator.hpp"
#include "../utils/logger.hpp"

namespace fss {

const Block                zero_block       = ToBlock(0, 0);
const Block                one_block        = ToBlock(0, 1);
const Block                all_one_block    = ToBlock(uint64_t(-1), uint64_t(-1));
const std::array<Block, 2> zero_and_all_one = {{zero_block, all_one_block}};

/**
 * @brief Sets the data of the block to a random value using SecureRng.
 *
 * Generates two 64-bit random numbers using SecureRng and sets them as the data of the block.
 */
void Block::SetRandom() {
    data = _mm_set_epi64x(tools::rng::SecureRng::Rand64(), tools::rng::SecureRng::Rand64());
}

/**
 * @brief Converts the data of the block to a uint32_t value of the specified bit size.
 *
 * Converts the data of the block to a 32-bit integer using _mm_cvtsi128_si32 and masks it to the specified bit size.
 *
 * @param bit_size The number of bits to mask the converted value with.
 * @return The converted uint32_t value.
 */
uint32_t Block::Convert(const uint32_t bit_size) const {
    return _mm_cvtsi128_si32(data) & ((1U << bit_size) - 1U);
}

/**
 * @brief Converts the block data into a vector of uint32_t values with the specified bit size.
 *
 * Converts the data of the block into a vector of 'num' elements, each having 'bit_size' bits.
 * Supported values for 'num' are 4, 8, 16, 32, 64, and 128.
 *
 * @param num The number of elements in the resulting vector. Must be 4, 8, 16, 32, 64, or 128.
 * @param bit_size The bit size of each element in the resulting vector.
 * @return A vector of uint32_t values containing the converted data.
 */
std::vector<uint32_t> Block::ConvertVec(const uint32_t num, const uint32_t bit_size) const {
    std::vector<uint32_t> res(num);

    if (num != 4 && num != 8 && num != 16 && num != 32 && num != 64 && num != 128) {
        utils::Logger::FatalLog(LOCATION, "Invalid num: " + std::to_string(num));
        exit(EXIT_FAILURE);
    }

    uint32_t mask = (1 << bit_size) - 1;

    if (num == 4) {
        // num = 4, each part is 32 bits
        for (int i = 0; i < 4; ++i) {
            res[i] = _mm_extract_epi32(data, i) & mask;
        }
    } else if (num == 8) {
        // num = 8, each part is 16 bits
        for (int i = 0; i < 8; ++i) {
            uint16_t val = (_mm_extract_epi16(data, i) & mask);
            res[i]       = val;
        }
    } else if (num == 16) {
        // num = 16, each part is 8 bits
        for (int i = 0; i < 16; ++i) {
            uint8_t val = (_mm_extract_epi8(data, i) & mask);
            res[i]      = val;
        }
    } else if (num == 32) {
        // num = 32, each part is 4 bits
        uint8_t bytes[16];
        _mm_store_si128((__m128i *)bytes, data);
        for (int i = 0; i < 32; ++i) {
            uint8_t byte = bytes[i / 2];
            res[i]       = (i % 2 == 0) ? (byte & 0x0F) & mask : (byte >> 4) & mask;
        }
    } else if (num == 64) {
        // num = 64, each part is 2 bits
        uint8_t bytes[16];
        _mm_store_si128((__m128i *)bytes, data);
        for (int i = 0; i < 64; ++i) {
            uint8_t byte = bytes[i / 4];
            res[i]       = (byte >> (2 * (i % 4))) & 0x03 & mask;
        }
    } else if (num == 128) {
        // num = 128, each part is 1 bit
        uint8_t bytes[16];
        _mm_store_si128((__m128i *)bytes, data);
        for (int i = 0; i < 128; ++i) {
            uint8_t byte = bytes[i / 8];
            res[i]       = (byte >> (i % 8)) & 0x01 & mask;
        }
    }
    return res;
}

/**
 * @brief Sets the block data from a vector of uint32_t values with the specified bit size.
 *
 * Sets the block data using the values from the provided vector 'vec'. The vector contains 'num' elements,
 * each with 'bit_size' bits. Supported values for 'num' are 32 and 64.
 *
 * @param vec The vector of uint32_t values to set the block data from.
 * @param num The number of elements in the vector. Must be 32 or 64.
 * @param bit_size The bit size of each element in the vector.
 */
void Block::FromVec(const std::vector<uint32_t> &vec, const uint32_t num, const uint32_t bit_size) {
    if (num != 32 && num != 64) {
        utils::Logger::FatalLog(LOCATION, "Invalid num: " + std::to_string(num));
        exit(EXIT_FAILURE);
    }

    uint32_t mask      = (1 << bit_size) - 1;
    uint8_t  bytes[16] = {0};
    if (num == 32) {
        for (int i = 0; i < 32; ++i) {
            if (i % 2 == 0) {
                bytes[i / 2] |= (vec[i] & mask);
            } else {
                bytes[i / 2] |= (vec[i] & mask) << 4;
            }
        }
    } else if (num == 64) {
        for (int i = 0; i < 64; ++i) {
            bytes[i / 4] |= (vec[i] & mask) << (2 * (i % 4));
        }
    }

    data = _mm_load_si128((__m128i *)bytes);
}

/**
 * @brief Prints the block data as a hexadecimal string for tracing purposes.
 *
 * Prints the block data as a hexadecimal string to the trace log, along with the specified message.
 * The log entry includes the file path, line number, and function name.
 *
 * @param file_path The file path from which the log entry is generated.
 * @param line_num The line number from which the log entry is generated.
 * @param func_name The function name from which the log entry is generated.
 * @param msg The message to include with the log entry.
 * @param debug If true, the log entry is generated; otherwise, no action is taken.
 */
void Block::PrintBlockHexTrace(const std::string &location, const std::string &msg, const bool debug) const {
    if (debug) {
        unsigned char buffer[16];
        _mm_storeu_si128((__m128i *)buffer, data);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 15; i >= 0; i--) {
            ss << std::setw(2) << static_cast<int>(buffer[i]);
            if (i > 0) {
                ss << " ";
            }
        }
        utils::Logger::TraceLog(location, msg + ss.str(), debug);
    }
}

/**
 * @brief Prints the block data as a binary string for tracing purposes.
 *
 * Prints the block data as a binary string to the trace log, along with the specified message.
 * The log entry includes the file path, line number, and function name.
 *
 * @param file_path The file path from which the log entry is generated.
 * @param line_num The line number from which the log entry is generated.
 * @param func_name The function name from which the log entry is generated.
 * @param msg The message to include with the log entry.
 * @param debug If true, the log entry is generated; otherwise, no action is taken.
 */
void Block::PrintBlockBinTrace(const std::string &location, const std::string &msg, const bool debug) const {
    if (debug) {
        unsigned char buffer[16];
        _mm_storeu_si128((__m128i *)buffer, data);

        std::stringstream ss;
        for (int i = 15; i >= 0; i--) {
            for (int j = 7; j >= 0; j--) {
                ss << ((buffer[i] >> j) & 1);
            }
            if (i > 0) {
                ss << " ";
            }
        }
        utils::Logger::TraceLog(location, msg + ss.str(), debug);
    }
}

/**
 * @brief Prints the block data as a hexadecimal string for debugging purposes.
 *
 * Prints the block data as a hexadecimal string to the debug log, along with the specified message.
 * The log entry includes the file path, line number, and function name.
 *
 * @param file_path The file path from which the log entry is generated.
 * @param line_num The line number from which the log entry is generated.
 * @param func_name The function name from which the log entry is generated.
 * @param msg The message to include with the log entry.
 * @param debug If true, the log entry is generated; otherwise, no action is taken.
 */
void Block::PrintBlockHexDebug(const std::string &location, const std::string &msg, const bool debug) const {
    if (debug) {
        unsigned char buffer[16];
        _mm_storeu_si128((__m128i *)buffer, data);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 15; i >= 0; i--) {
            ss << std::setw(2) << static_cast<int>(buffer[i]);
            if (i > 0) {
                ss << " ";
            }
        }
        utils::Logger::DebugLog(location, msg + ss.str(), debug);
    }
}

/**
 * @brief Prints the block data as a binary string for debugging purposes.
 *
 * Prints the block data as a binary string to the debug log, along with the specified message.
 * The log entry includes the file path, line number, and function name.
 *
 * @param file_path The file path from which the log entry is generated.
 * @param line_num The line number from which the log entry is generated.
 * @param func_name The function name from which the log entry is generated.
 * @param msg The message to include with the log entry.
 * @param debug If true, the log entry is generated; otherwise, no action is taken.
 */
void Block::PrintBlockBinDebug(const std::string &location, const std::string &msg, const bool debug) const {
    if (debug) {
        unsigned char buffer[16];
        _mm_storeu_si128((__m128i *)buffer, data);

        std::stringstream ss;
        for (int i = 15; i >= 0; i--) {
            for (int j = 7; j >= 0; j--) {
                ss << ((buffer[i] >> j) & 1);
            }
            if (i > 0) {
                ss << " ";
            }
        }
        utils::Logger::DebugLog(location, msg + ss.str(), debug);
    }
}

}    // namespace fss
