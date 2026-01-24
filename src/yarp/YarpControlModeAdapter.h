// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPCONTROLMODEADAPTER_H
#define DINRAIL_YARPCONTROLMODEADAPTER_H

#include <dinrail/IControlMode.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IControlMode.h>

namespace dinrail
{

/**
 * @brief Adapter that converts yarp::dev::IControlMode to dinrail::IControlMode.
 */
class YarpControlModeAdapter : public IControlMode
{
public:
    explicit YarpControlModeAdapter(yarp::dev::PolyDriver* yarpDevice);
    
    bool getControlMode(int j, int *mode) override;
    bool getControlModes(int *modes) override;
    bool getControlModes(const int n_joint, const int *joints, int *modes) override;
    bool setControlMode(const int j, const int mode) override;
    bool setControlModes(const int n_joint, const int *joints, int *modes) override;
    bool setControlModes(int *modes) override;

private:
    yarp::dev::IControlMode* m_yarpInterface{nullptr};
};

} // namespace dinrail

#endif // DINRAIL_YARPCONTROLMODEADAPTER_H
