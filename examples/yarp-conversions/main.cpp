// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Parameters.h>
#include <dinrail/YarpPropertyConverter.h>

#include <yarp/os/Property.h>

#include <iostream>
#include <string>

int main()
{
    // Build a minimal dinrail::Parameters object.
    dinrail::Parameters dinrailParams;
    dinrailParams.put("device", "fakeMotionControl");
    dinrailParams.put("period_ms", 10);

    dinrail::Parameters& general = dinrailParams.addGroup("GENERAL");
    general.put("joints", 2);

    // Convert dinrail::Parameters -> yarp::os::Property.
    yarp::os::Property yarpProperty = dinrail::YarpPropertyConverter::toYarpProperty(dinrailParams);
    std::cout << "YARP property: " << yarpProperty.toString() << "\n";

    // Convert yarp::os::Property -> dinrail::Parameters.
    dinrail::Parameters roundTrip = dinrail::YarpPropertyConverter::toDinrailParameters(yarpProperty);

    std::string device;
    int periodMs = 0;
    int joints = 0;

    if (!roundTrip.getParameter("device", device) || !roundTrip.getParameter("period_ms", periodMs))
    {
        std::cerr << "Failed to read round-trip top-level parameters.\n";
        return 1;
    }

    const dinrail::Parameters& roundTripGeneral = roundTrip.findGroup("GENERAL");
    if (roundTripGeneral.isNull() || !roundTripGeneral.getParameter("joints", joints))
    {
        std::cerr << "Failed to read round-trip GENERAL group.\n";
        return 1;
    }

    std::cout << "Round-trip values:" << "\n"
              << "  device: " << device << "\n"
              << "  period_ms: " << periodMs << "\n"
              << "  GENERAL.joints: " << joints << "\n";

    return 0;
}
