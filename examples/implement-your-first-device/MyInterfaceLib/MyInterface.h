// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MYINTERFACE_H
#define MYINTERFACE_H

namespace myexample
{

/// Example of a custom interface exposed by a dinrail device.
class MyInterface
{
public:
    virtual ~MyInterface() = default;

    virtual bool setGain(double gain) = 0;
    virtual bool getGain(double& gain) = 0;
};

} // namespace myexample

#endif // MYINTERFACE_H
