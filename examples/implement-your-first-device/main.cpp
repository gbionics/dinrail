// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "MyInterface.h"

#include <dinrail/Device.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/Parameters.h>

#include <iostream>

int main()
{
    // Configure the device.
    // "device" must match the target suffix in dinrail_add_device(...).
    dinrail::Parameters opts;
    opts.put("device", "MyDevice");
    opts.put("number_of_joints", 3);
    opts.put("joint_names", std::vector<std::string>{"shoulder", "elbow", "wrist"});
    opts.put("gain", 1.5);

    // Open the device. dinrail::Device locates and loads the shared library
    // "dinrail-device-MyDevice" at runtime.
    dinrail::Device dev;
    if (!dev.open(opts))
    {
        std::cerr << "Failed to open MyDevice.\n";
        return 1;
    }

    // Query the IAxisInfo interface.
    // view<T>() works because MyDevice directly inherits dinrail::IAxisInfo.
    dinrail::IAxisInfo* axisInfo = nullptr;
    if (!dev.view(axisInfo) || axisInfo == nullptr)
    {
        std::cerr << "IAxisInfo not available.\n";
        dev.close();
        return 1;
    }

    int axes = 0;
    axisInfo->getAxes(&axes);
    std::cout << "Opened MyDevice with " << axes << " axis(es):\n";

    // Query and use a custom interface exposed by the same device.
    myexample::MyInterface* custom = nullptr;
    if (!dev.view(custom) || custom == nullptr)
    {
        std::cerr << "MyInterface not available.\n";
        dev.close();
        return 1;
    }

    double gain = 0.0;
    custom->getGain(gain);
    std::cout << "Initial gain: " << gain << "\n";

    custom->setGain(2.0);
    custom->getGain(gain);
    std::cout << "Updated gain: " << gain << "\n";

    for (int j = 0; j < axes; ++j)
    {
        std::string name;
        dinrail::JointType type = dinrail::JointType::UNKNOWN;
        axisInfo->getAxisName(j, name);
        axisInfo->getJointType(j, type);

        const char* typeStr =
            (type == dinrail::JointType::REVOLUTE) ? "revolute"
            : (type == dinrail::JointType::PRISMATIC) ? "prismatic"
                                                      : "unknown";
        std::cout << "  [" << j << "] " << name << " (" << typeStr << ")\n";
    }

    dev.close();
    return 0;
}
