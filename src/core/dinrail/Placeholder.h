// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_PLACEHOLDER_H
#define DINRAIL_PLACEHOLDER_H

namespace dinrail
{

/**
 * @brief Minimal placeholder class used to keep the core library non-empty.
 */
class Placeholder
{
public:
    Placeholder() = default;
    ~Placeholder() = default;

    bool isPlaceholder() const;
};

} // namespace dinrail

#endif // DINRAIL_PLACEHOLDER_H
