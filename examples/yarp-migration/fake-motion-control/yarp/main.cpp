#include <yarp/dev/IAxisInfo.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Property.h>

#include <iostream>

int main()
{
    yarp::os::Property opts;
    opts.put("device", "fakeMotionControl");

    yarp::os::Property& grp = opts.addGroup("GENERAL");
    grp.put("Joints", 6);

    yarp::dev::PolyDriver drv;
    if (!drv.open(opts))
    {
        std::cerr << "Failed to open device 'fakeMotionControl'.\n";
        return 1;
    }

    yarp::dev::IAxisInfo* axisInfo = nullptr;
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
        yarp::dev::JointTypeEnum axisType = yarp::dev::VOCAB_JOINTTYPE_UNKNOWN;

        if (!axisInfo->getAxisName(j, axisName) || !axisInfo->getJointType(j, axisType))
        {
            std::cerr << "Failed to read axis info for joint " << j << "\n";
            drv.close();
            return 1;
        }

        std::cout << "Axis " << j << ": name='" << axisName << "', type=" << axisType << "\n";
    }

    drv.close();
    return 0;
}
