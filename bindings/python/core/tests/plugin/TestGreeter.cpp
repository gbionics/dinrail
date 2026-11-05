// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "TestGreeter.h"

bool TestGreeter::open(const dinrail::Parameters& config)
{
    if (config.check("greeting"))
    {
        return config.getParameter("greeting", m_greeting);
    }
    return true;
}

bool TestGreeter::close()
{
    return true;
}

std::string TestGreeter::greet(const std::string& name)
{
    return m_greeting + " " + name;
}
