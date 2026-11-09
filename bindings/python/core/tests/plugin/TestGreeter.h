// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_BINDINGS_TEST_TESTGREETER_H
#define DINRAIL_BINDINGS_TEST_TESTGREETER_H

#include "IGreeter.h"

#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>

#include <string>

/**
 * @brief Device plugin implementing an interface not known by dinrail.
 */
class TestGreeter final : public dinrail::IDevice, public IGreeter
{
public:
    bool open(const dinrail::Parameters& config) override;
    bool close() override;

    std::string greet(const std::string& name) override;

private:
    std::string m_greeting{"Hello"};
};

#endif // DINRAIL_BINDINGS_TEST_TESTGREETER_H
