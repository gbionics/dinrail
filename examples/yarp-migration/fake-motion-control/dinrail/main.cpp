#include <dinrail/Device.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/Parameters.h>

#include <CLI/CLI.hpp>

#include <iostream>

int main(int argc, char** argv)
{
    CLI::App app{"Dinrail fake motion control IAxisInfo example"};

    bool onlyUseYarpDevice = false;
    app.add_flag("--only-use-yarp-device",
                 onlyUseYarpDevice,
                 "Force loading the YARP device instead of trying dinrail device first");

    CLI11_PARSE(app, argc, argv);

    dinrail::Parameters opts;
    opts.put("device", "fakeMotionControl");
    opts.put("dinrail_device_type", onlyUseYarpDevice ? "yarp" : "dinrail");

    dinrail::Parameters& grp = opts.addGroup("GENERAL");
    grp.put("Joints", 6);

    dinrail::Device drv;
    if (!drv.open(opts))
    {
        std::cerr << "Failed to open device 'fakeMotionControl'.\n";
        return 1;
    }

    dinrail::IAxisInfo* axisInfo = nullptr;
    if (!drv.view(axisInfo) || axisInfo == nullptr)
    {
        std::cerr << "IAxisInfo not available.\n";
        drv.close();
        return 1;
    }

    int axes = 0;
    if (!axisInfo->getAxes(&axes) || axes <= 0)
    {
        std::cerr << "getAxes() failed.\n";
        drv.close();
        return 1;
    }

    std::cout << "fakeMotionControl opened. Axes = " << axes << "\n";
    for (int j = 0; j < axes; ++j)
    {
        std::string axisName;
        dinrail::JointType axisType = dinrail::JointType::UNKNOWN;

        if (!axisInfo->getAxisName(j, axisName) || !axisInfo->getJointType(j, axisType))
        {
            std::cerr << "Failed to read axis info for joint " << j << "\n";
            drv.close();
            return 1;
        }

        std::cout << "Axis " << j << ": name='" << axisName
                  << "', type=" << static_cast<int>(axisType) << "\n";
    }

    drv.close();
    return 0;
}
