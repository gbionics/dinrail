#include <dinrail/Parameters.h>

#include <dinrail/Device.h>
#include <dinrail/IPositionDirect.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IControlMode.h>

#include <CLI/CLI.hpp>

#include <iostream>
#include <vector>
#include <iomanip>

static void printVec(const std::vector<double>& v, int prec = 2)
{
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << std::fixed << std::setprecision(prec) << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]";
}

int main(int argc, char** argv)
{
    CLI::App app{"Dinrail fake motion control example"};
    
    bool onlyUseYarpDevice = false;
    app.add_flag("--only-use-yarp-device", onlyUseYarpDevice,
                 "Force loading the YARP device instead of trying dinrail device first");
    
    CLI11_PARSE(app, argc, argv);

    // Configure fakeMotionControl
    dinrail::Parameters opts;
    opts.put("device", "fakeMotionControl");

    // By default given a device name, dinrail first tries to load a dinrail device if it can finds it,
    // otherwise it falls back to YARP device. By passing --only-use-yarp-device it is possible to force dinrail
    // to load the fakeMotionControl YARP device directly.
    if (onlyUseYarpDevice)
    {
        opts.put("dinrail_device_type", "yarp");
    } 
    else 
    {
        // This is the default
        opts.put("dinrail_device_type", "dinrail");
    }

    dinrail::Parameters& grp = opts.addGroup("GENERAL");
    grp.put("Joints", 6);

    dinrail::Device drv;
    if (!drv.open(opts)) {
        std::cerr << "Failed to open device 'fakeMotionControl'.\n"
                  << "Hint: ensure the fakeMotionControl plugin is available in your YARP installation, and that dinrail has been compiled with YARP support.\n";
        return 1;
    }

    dinrail::IPositionDirect* iposd = nullptr;
    dinrail::IEncoders* ienc = nullptr;
    dinrail::IControlMode* icm = nullptr;

    if (!drv.view(iposd) || !iposd) {
        std::cerr << "IPositionDirect not available.\n";
        drv.close();
        return 1;
    }
    if (!drv.view(ienc) || !ienc) {
        std::cerr << "IEncoders not available.\n";
        drv.close();
        return 1;
    }
    drv.view(icm); // optional

    int axes = 0;
    if (!iposd->getAxes(&axes) || axes <= 0) {
        std::cerr << "getAxes() failed.\n";
        drv.close();
        return 1;
    }

    std::cout << "fakeMotionControl opened. Axes = " << axes << "\n";

    // Optional: set POSITION_DIRECT mode (recommended on real devices).
    if (icm) {
        for (int j = 0; j < axes; ++j) {
            if (!icm->setControlMode(j, dinrail::VOCAB_CM_POSITION_DIRECT)) {
                std::cerr << "Warning: setControlMode(POSITION_DIRECT) failed on joint " << j << "\n";
            }
        }
        std::cout << "Requested POSITION_DIRECT mode for all joints.\n";
    } else {
        std::cout << "IControlMode not available; continuing anyway.\n";
    }

    // Read encoders BEFORE
    std::vector<double> encBefore(static_cast<size_t>(axes), 0.0);
    if (ienc->getEncoders(encBefore.data())) {
        std::cout << "Encoders (before) = ";
        printVec(encBefore);
        std::cout << "\n";
    } else {
        std::cerr << "getEncoders(before) failed.\n";
    }

    // Send a couple of direct position commands (no trajectory generator).
    iposd->setPosition(0, 10.0);
    if (axes > 1) iposd->setPosition(1, -5.0);

    // Read back the last commanded references (this should reflect setPosition calls).
    std::vector<double> refs(static_cast<size_t>(axes), 0.0);
    if (iposd->getRefPositions(refs.data())) {
        std::cout << "Ref positions (after setPosition) = ";
        printVec(refs);
        std::cout << "\n";
    } else {
        std::cerr << "getRefPositions() failed.\n";
    }

    // Read encoders AFTER
    std::vector<double> encAfter(static_cast<size_t>(axes), 0.0);
    if (ienc->getEncoders(encAfter.data())) {
        std::cout << "Encoders (after)  = ";
        printVec(encAfter);
        std::cout << "\n";
    } else {
        std::cerr << "getEncoders(after) failed.\n";
    }

    drv.close();
    std::cout << "Closed.\n";
    return 0;
}
