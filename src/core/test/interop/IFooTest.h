// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_TEST_IFOOTEST_H
#define DINRAIL_TEST_IFOOTEST_H

#include <string>

namespace dinrail::test
{

// A custom interface that dinrail does not know about, used to demonstrate that
// an interop plugin can add new interfaces resolvable through Device::view().
class IFooTest
{
public:
    virtual ~IFooTest() = default;

    virtual std::string tag() const = 0;
};

// Destination interface used to verify runtime interop translations in core.
class ITranslatedFooTest
{
public:
    virtual ~ITranslatedFooTest() = default;
    virtual std::string translatedTag() const = 0;
};

} // namespace dinrail::test

#endif // DINRAIL_TEST_IFOOTEST_H
