// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "Vocab.h"

namespace dinrail
{

namespace Vocab32
{

std::string decode(vocab32_t code)
{
    std::string result;
    result.reserve(4);

    for (int i = 0; i < 4; ++i)
    {
        char c = static_cast<char>((code >> (i * 8)) & 0xFF);
        if (c != 0)
        {
            result += c;
        }
    }

    return result;
}

vocab32_t encode(const std::string& str)
{
    vocab32_t result = 0;

    for (size_t i = 0; i < str.size() && i < 4; ++i)
    {
        result |= static_cast<vocab32_t>(static_cast<unsigned char>(str[i])) << (i * 8);
    }

    return result;
}

} // namespace Vocab32

} // namespace dinrail
