#include <yarp/os/Network.h>
#include <yarp/os/Property.h>

#include <yarp/dev/IControlMode.h> // VOCAB_CM_POSITION_DIRECT
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IPositionDirect.h>
#include <yarp/dev/PolyDriver.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

static void printVec(const std::vector<double>& v, int prec = 2)
{
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i)
    {
        std::cout << std::fixed << std::setprecision(prec) << v[i];
        if (i + 1 < v.size())
            std::cout << ", ";
    }
    std::cout << "]";
}

int main()
{
    // Configure fakeMotionControl
    yarp::os::Property opts;
    opts.put("device", "fakeMotionControl");

    yarp::os::Property& grp = opts.addGroup("GENERAL");
    grp.put("Joints", 6);

    yarp::dev::PolyDriver drv;
    if (!drv.open(opts))
    {
        std::cerr << "Failed to open device 'fakeMotionControl'.\n"
                  << "Hint: ensure the fakeMotionControl plugin is available in your YARP "
                     "installation.\n";
        return 1;
    }

    yarp::dev::IPositionDirect* iposd = nullptr;
    yarp::dev::IEncoders* ienc = nullptr;
    yarp::dev::IControlMode* icm = nullptr;

    if (!drv.view(iposd) || !iposd)
    {
        std::cerr << "IPositionDirect not available.\n";
        drv.close();
        return 1;
    }
    if (!drv.view(ienc) || !ienc)
    {
        std::cerr << "IEncoders not available.\n";
        drv.close();
        return 1;
    }
    drv.view(icm); // optional

    int axes = 0;
    if (!iposd->getAxes(&axes) || axes <= 0)
    {
        std::cerr << "getAxes() failed.\n";
        drv.close();
        return 1;
    }

    std::cout << "fakeMotionControl opened. Axes = " << axes << "\n";

    // Optional: set POSITION_DIRECT mode (recommended on real devices).
    if (icm)
    {
        for (int j = 0; j < axes; ++j)
        {
            if (!icm->setControlMode(j, VOCAB_CM_POSITION_DIRECT))
            {
                std::cerr << "Warning: setControlMode(POSITION_DIRECT) failed on joint " << j
                          << "\n";
            }
        }
        std::cout << "Requested POSITION_DIRECT mode for all joints.\n";
    } else
    {
        std::cout << "IControlMode not available; continuing anyway.\n";
    }

    // Read encoders BEFORE
    std::vector<double> encBefore(static_cast<size_t>(axes), 0.0);
    if (ienc->getEncoders(encBefore.data()))
    {
        std::cout << "Encoders (before) = ";
        printVec(encBefore);
        std::cout << "\n";
    } else
    {
        std::cerr << "getEncoders(before) failed.\n";
    }

    // Send a couple of direct position commands (no trajectory generator).
    iposd->setPosition(0, 10.0);
    if (axes > 1)
        iposd->setPosition(1, -5.0);

    // Read back the last commanded references (this should reflect setPosition calls).
    std::vector<double> refs(static_cast<size_t>(axes), 0.0);
    if (iposd->getRefPositions(refs.data()))
    {
        std::cout << "Ref positions (after setPosition) = ";
        printVec(refs);
        std::cout << "\n";
    } else
    {
        std::cerr << "getRefPositions() failed.\n";
    }

    // Wait for 20 ms as the default update period of fakeMotionControl is 10 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // Read encoders AFTER (may or may not change instantly depending on device behavior)
    std::vector<double> encAfter(static_cast<size_t>(axes), 0.0);
    if (ienc->getEncoders(encAfter.data()))
    {
        std::cout << "Encoders (after)  = ";
        printVec(encAfter);
        std::cout << "\n";
    } else
    {
        std::cerr << "getEncoders(after) failed.\n";
    }

    drv.close();
    std::cout << "Closed.\n";
    return 0;
}
