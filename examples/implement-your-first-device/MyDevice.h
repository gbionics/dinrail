// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MYDEVICE_H
#define MYDEVICE_H

#include "MyInterface.h"

#include <dinrail/IAxisInfo.h>
#include <dinrail/IDevice.h>

#include <string>
#include <vector>

/// A minimal dinrail device that exposes the IAxisInfo interface.
///
/// This class is the entry point for implementing your own device.
/// It inherits from dinrail::IDevice (required) and from any dinrail
/// interface that the device supports. In this example, we also implement IAxisInfo to provide metadata about the device axes. You can implement as many interfaces as needed, and they will be automatically exposed by the plugin system.
class MyDevice : public dinrail::IDevice, public dinrail::IAxisInfo, public myexample::MyInterface
{
public:
    // ---- dinrail::IDevice ----

    bool open(const dinrail::Parameters& config) override;
    bool close() override;

    // ---- dinrail::IAxisInfo ----

    bool getAxes(int* ax) override;
    bool getAxisName(int axis, std::string& name) override;
    bool getJointType(int axis, dinrail::JointType& type) override;

    // ---- myexample::MyInterface ----

    bool setGain(double gain) override;
    bool getGain(double& gain) override;

private:
    int m_njoints{0};
    double m_gain{1.0};
    std::vector<std::string> m_axisNames;
};

#endif // MYDEVICE_H
