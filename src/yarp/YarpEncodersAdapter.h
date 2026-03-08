// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPENCODERSADAPTER_H
#define DINRAIL_YARPENCODERSADAPTER_H

#include <dinrail/IEncoders.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/PolyDriver.h>

namespace dinrail
{

/**
 * @brief Adapter that converts yarp::dev::IEncoders to dinrail::IEncoders.
 */
class YarpEncodersAdapter : public IEncoders
{
public:
    explicit YarpEncodersAdapter(yarp::dev::PolyDriver* yarpDevice);

    bool getAxes(int* ax) override;
    bool resetEncoder(int j) override;
    bool resetEncoders() override;
    bool setEncoder(int j, double val) override;
    bool setEncoders(const double* vals) override;
    bool getEncoder(int j, double* v) override;
    bool getEncoders(double* encs) override;
    bool getEncoderSpeed(int j, double* sp) override;
    bool getEncoderSpeeds(double* spds) override;
    bool getEncoderAcceleration(int j, double* spds) override;
    bool getEncoderAccelerations(double* accs) override;

private:
    yarp::dev::IEncoders* m_yarpInterface{nullptr};
};

} // namespace dinrail

#endif // DINRAIL_YARPENCODERSADAPTER_H
