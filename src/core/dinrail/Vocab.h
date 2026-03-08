// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_VOCAB_H
#define DINRAIL_VOCAB_H

#include <cstdint>
#include <string>

namespace dinrail
{

/**
 * @brief Type for vocabulary identifiers (32-bit).
 *
 * Compatible with YARP's vocab32_t.
 */
using vocab32_t = std::int32_t;

/**
 * @brief Create a vocab from characters.
 *
 * Creates a 32-bit integer from up to 4 characters, packed little-endian.
 * This encoding is compatible with YARP's createVocab32.
 *
 * Example: createVocab32('s','e','t') creates a readable integer code.
 *
 * @param a first character of the vocab
 * @param b second character of the vocab (default 0)
 * @param c third character of the vocab (default 0)
 * @param d fourth character of the vocab (default 0)
 * @return 32-bit vocabulary identifier
 */
constexpr vocab32_t createVocab32(char a, char b = 0, char c = 0, char d = 0)
{
    return (static_cast<vocab32_t>(a)) + (static_cast<vocab32_t>(b) << 8)
           + (static_cast<vocab32_t>(c) << 16) + (static_cast<vocab32_t>(d) << 24);
}

/**
 * @brief Vocabulary utilities.
 */
namespace Vocab32
{
/**
 * @brief Convert a vocabulary identifier into a string.
 *
 * @param code the vocabulary identifier to convert
 * @return the string representation (up to 4 characters)
 */
std::string decode(vocab32_t code);

/**
 * @brief Convert a string into a vocabulary identifier.
 *
 * If the string is longer than four characters, only the first
 * four characters are used.
 *
 * @param str the string to convert
 * @return the integer equivalent of the string
 */
vocab32_t encode(const std::string& str);
} // namespace Vocab32

} // namespace dinrail

#endif // DINRAIL_VOCAB_H
