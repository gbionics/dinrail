// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPPOSITIONDIRECTADAPTER_H
#define DINRAIL_YARPPOSITIONDIRECTADAPTER_H

#include <dinrail/IPositionDirect.h>
#include <yarp/dev/IPositionDirect.h>
#include <yarp/dev/PolyDriver.h>

namespace dinrail
{

/**
 * @brief Adapter that converts yarp::dev::IPositionDirect to dinrail::IPositionDirect.
 */
class YarpPositionDirectAdapter : public IPositionDirect
{
public:
    explicit YarpPositionDirectAdapter(yarp::dev::PolyDriver* yarpDevice);

    bool getAxes(int* ax) override;
    bool setPosition(int j, double ref) override;
    bool setPositions(const int n_joint, const int* joints, const double* refs) override;
    bool setPositions(const double* refs) override;
    bool getRefPosition(const int joint, double* ref) override;
    bool getRefPositions(double* refs) override;
    bool getRefPositions(const int n_joint, const int* joints, double* refs) override;

private:
    yarp::dev::IPositionDirect* m_yarpInterface{nullptr};
};

} // namespace dinrail

#endif // DINRAIL_YARPPOSITIONDIRECTADAPTER_H
