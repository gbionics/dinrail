// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_BINDINGS_TEST_IGREETER_H
#define DINRAIL_BINDINGS_TEST_IGREETER_H

#include <string>

/**
 * @brief Interface not known by dinrail, used to test the registration of external interfaces.
 */
class IGreeter
{
public:
    virtual ~IGreeter();

    virtual std::string greet(const std::string& name) = 0;
};

#endif // DINRAIL_BINDINGS_TEST_IGREETER_H
