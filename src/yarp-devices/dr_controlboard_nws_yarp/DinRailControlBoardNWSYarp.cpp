/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-FileCopyrightText: 2006-2010 RobotCub Consortium
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "DinRailControlBoardNWSYarp.h"

#include "ControlBoardLogComponent.h"
#include "RPCMessagesParser.h"
#include "StreamingMessagesParser.h"

#include <dinrail/ControlBoardYARPJointData.h>

#include <cmath>
#include <numeric>

using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using dinrail::ControlBoardYARPJointData;

const double DEFAULT_PERIOD = 0.02;

DinRailControlBoardNWSYarp::DinRailControlBoardNWSYarp()
    : yarp::os::PeriodicThread(DEFAULT_PERIOD)
{
}

void DinRailControlBoardNWSYarp::closePorts()
{
    inputRPCPort.interrupt();
    inputRPCPort.removeCallbackLock();
    inputRPCPort.close();

    inputStreamingPort.interrupt();
    inputStreamingPort.close();

    outputPositionStatePort.interrupt();
    outputPositionStatePort.close();

    extendedOutputStatePort.interrupt();
    extendedOutputStatePort.close();
}

bool DinRailControlBoardNWSYarp::close()
{
    // Ensure that the device is not running
    if (isRunning())
    {
        stop();
    }

    closeDevice();
    closePorts();

    return true;
}

bool DinRailControlBoardNWSYarp::open(Searchable& config)
{
    if (!parseParams(config))
    {
        return false;
    }

    std::string rootName = m_name + m_namesuffix;

    // Open ports, then attach the readers or callbacks
    if (!inputRPCPort.open((rootName + "/rpc:i")))
    {
        controlBoardLogger().error("Error opening port {}", rootName + "/rpc:i");
        closePorts();
        return false;
    }
    inputRPCPort.setReader(RPC_parser);
    inputRPC_buffer.attach(inputRPCPort);
    RPC_parser.attach(inputRPC_buffer);

    if (!inputStreamingPort.open(rootName + "/command:i"))
    {
        controlBoardLogger().error("Error opening port {}", rootName + "/rpc:i");
        closePorts();
        return false;
    }
    inputStreamingPort.setStrict();
    inputStreamingPort.useCallback(streaming_parser);

    if (!outputPositionStatePort.open(rootName + "/state:o"))
    {
        controlBoardLogger().error("Error opening port {}", rootName + "/state:o");
        closePorts();
        return false;
    }

    // Extended output state port
    if (!extendedOutputStatePort.open(rootName + "/stateExt:o"))
    {
        controlBoardLogger().error("Error opening port {}", rootName + "/state:o");
        closePorts();
        return false;
    }
    extendedOutputState_buffer.attach(extendedOutputStatePort);

    // In case attach is not deferred and the controlboard already owns a valid device
    // we can start the thread. Otherwise this will happen when attachAll is called
    if (subdevice_ready)
    {
        setPeriod(m_period);
        if (!start())
        {
            controlBoardLogger().error("Error starting thread");
            return false;
        }
    }

    return true;
}

bool DinRailControlBoardNWSYarp::setDevice(yarp::dev::DeviceDriver* driver, bool owned)
{
    // Save the pointer and subDeviceOwned
    yarp::dev::DeviceDriver* subdevice_ptr = driver;

    // yarp::dev::IJointFault* iJointFault{nullptr};
    subdevice_ptr->view(iJointFault);
    if (!iJointFault)
    {
        controlBoardLogger().warn("Part <%s>: iJointFault interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IPidControl* iPidControl{nullptr};
    subdevice_ptr->view(iPidControl);
    if (!iPidControl)
    {
        controlBoardLogger().warn("Part <%s>: IPidControl interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IPositionControl* iPositionControl{nullptr};
    subdevice_ptr->view(iPositionControl);
    if (!iPositionControl)
    {
        controlBoardLogger().warn("Part <%s>: IPositionControl interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IPositionDirect* iPositionDirect{nullptr};
    subdevice_ptr->view(iPositionDirect);
    if (!iPositionDirect)
    {
        controlBoardLogger().warn("Part <%s>: IPositionDirect interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IVelocityControl* iVelocityControl{nullptr};
    subdevice_ptr->view(iVelocityControl);
    if (!iVelocityControl)
    {
        controlBoardLogger().warn("Part <%s>: IVelocityControl interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IEncodersTimed* iEncodersTimed{nullptr};
    subdevice_ptr->view(iEncodersTimed);
    if (!iEncodersTimed)
    {
        controlBoardLogger().warn("Part <%s>: IEncodersTimed interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IMotor* iMotor{nullptr};
    subdevice_ptr->view(iMotor);
    if (!iMotor)
    {
        controlBoardLogger().warn("Part <%s>: IMotor interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IMotorEncoders* iMotorEncoders{nullptr};
    subdevice_ptr->view(iMotorEncoders);
    if (!iMotorEncoders)
    {
        controlBoardLogger().warn("Part <%s>: IMotorEncoders interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IAmplifierControl* iAmplifierControl{nullptr};
    subdevice_ptr->view(iAmplifierControl);
    if (!iAmplifierControl)
    {
        controlBoardLogger().warn("Part <%s>: IAmplifierControl interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IControlLimits* iControlLimits{nullptr};
    subdevice_ptr->view(iControlLimits);
    if (!iControlLimits)
    {
        controlBoardLogger().warn("Part <%s>: IControlLimits interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IControlCalibration* iControlCalibration{nullptr};
    subdevice_ptr->view(iControlCalibration);
    if (!iControlCalibration)
    {
        controlBoardLogger().warn("Part <%s>: IControlCalibration interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::ITorqueControl* iTorqueControl{nullptr};
    subdevice_ptr->view(iTorqueControl);
    if (!iTorqueControl)
    {
        controlBoardLogger().warn("Part <%s>: ITorqueControl interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IImpedanceControl* iImpedanceControl{nullptr};
    subdevice_ptr->view(iImpedanceControl);
    if (!iImpedanceControl)
    {
        controlBoardLogger().warn("Part <%s>: IImpedanceControl interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // dinrail::IImpedanceAllSetPointsControl* iImpedanceAllSetPointsControl{nullptr};
    subdevice_ptr->view(iImpedanceAllSetPointsControl);
    if (!iImpedanceAllSetPointsControl)
    {
        controlBoardLogger().warn("Part <%s>: IImpedanceAllSetPointsControl interface was not "
                                  "found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IControlMode* iControlMode{nullptr};
    subdevice_ptr->view(iControlMode);
    if (!iControlMode)
    {
        controlBoardLogger().warn("Part <%s>: IControlMode interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IAxisInfo* iAxisInfo{nullptr};
    subdevice_ptr->view(iAxisInfo);
    if (!iAxisInfo)
    {
        controlBoardLogger().warn("Part <%s>: IAxisInfo interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IPreciselyTimed* iPreciselyTimed{nullptr};
    subdevice_ptr->view(iPreciselyTimed);
    if (!iPreciselyTimed)
    {
        controlBoardLogger().warn("Part <%s>: IPreciselyTimed interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IInteractionMode* iInteractionMode{nullptr};
    subdevice_ptr->view(iInteractionMode);
    if (!iInteractionMode)
    {
        controlBoardLogger().warn("Part <%s>: IInteractionMode interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IRemoteVariables* iRemoteVariables{nullptr};
    subdevice_ptr->view(iRemoteVariables);
    if (!iRemoteVariables)
    {
        controlBoardLogger().warn("Part <%s>: IRemoteVariables interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::IPWMControl* iPWMControl{nullptr};
    subdevice_ptr->view(iPWMControl);
    if (!iPWMControl)
    {
        controlBoardLogger().warn("Part <%s>: IPWMControl interface was not found in subdevice.",
                                  partName.c_str());
    }

    // yarp::dev::ICurrentControl* iCurrentControl{nullptr};
    subdevice_ptr->view(iCurrentControl);
    if (!iCurrentControl)
    {
        controlBoardLogger().warn("Part <%s>: ICurrentControl interface was not found in "
                                  "subdevice.",
                                  partName.c_str());
    }

    // Get the number of controlled joints
    int tmp_axes = 0;
    if (iAxisInfo)
    {
        if (!iAxisInfo->getAxes(&tmp_axes))
        {
            controlBoardLogger().error("Part <{}>: iAxisInfo->getAxes() failed for subdevice",
                                       partName);
            return false;
        }
    } else if (iEncodersTimed)
    {
        if (!iEncodersTimed->getAxes(&tmp_axes))
        {
            controlBoardLogger().error("Part <{}>: iEncodersTimed->getAxes() failed for subdevice",
                                       partName);
            return false;
        }
    } else if (iPositionControl)
    {
        if (!iPositionControl->getAxes(&tmp_axes))
        {
            controlBoardLogger().error("Part <{}>: iPositionControl->getAxes() failed for "
                                       "subdevice",
                                       partName);
            return false;
        }
    } else if (iVelocityControl)
    {
        if (!iVelocityControl->getAxes(&tmp_axes))
        {
            controlBoardLogger().error("Part <{}>: iVelocityControl->getAxes() failed for "
                                       "subdevice",
                                       partName);
            return false;
        }
    }

    if (tmp_axes <= 0)
    {
        controlBoardLogger().error("Part <%s>: attached device has an invalid number of joints "
                                   "(%d)",
                                   partName.c_str(),
                                   tmp_axes);
        return false;
    }
    subdevice_joints = static_cast<size_t>(tmp_axes);
    times.resize(subdevice_joints);

    // Initialization
    streaming_parser.setImpedanceAllSetPointsControlEmulation(
        m_emulate_impedance_all_setpoints_control);
    streaming_parser.init(subdevice_ptr);
    streaming_parser.initialize();

    RPC_parser.setImpedanceAllSetPointsControlEmulation(m_emulate_impedance_all_setpoints_control);
    RPC_parser.init(subdevice_ptr);
    RPC_parser.initialize();

    return true;
}

void DinRailControlBoardNWSYarp::closeDevice()
{
    // Reset callbacks
    streaming_parser.reset();
    RPC_parser.reset();

    subdevice_joints = 0;
    subdevice_ready = false;

    times.clear();

    // Clear all interfaces
    iPidControl = nullptr;
    iPositionControl = nullptr;
    iPositionDirect = nullptr;
    iVelocityControl = nullptr;
    iEncodersTimed = nullptr;
    iMotor = nullptr;
    iMotorEncoders = nullptr;
    iAmplifierControl = nullptr;
    iControlLimits = nullptr;
    iControlCalibration = nullptr;
    iTorqueControl = nullptr;
    iImpedanceControl = nullptr;
    iImpedanceAllSetPointsControl = nullptr;
    iControlMode = nullptr;
    iAxisInfo = nullptr;
    iPreciselyTimed = nullptr;
    iInteractionMode = nullptr;
    iRemoteVariables = nullptr;
    iPWMControl = nullptr;
    iCurrentControl = nullptr;
    iJointFault = nullptr;
}

bool DinRailControlBoardNWSYarp::attach(yarp::dev::PolyDriver* poly)
{
    // Check if we already instantiated a subdevice previously
    if (subdevice_ready)
    {
        return false;
    }

    if (!setDevice(poly, false))
    {
        return false;
    }

    setPeriod(m_period);
    if (!start())
    {
        controlBoardLogger().error("Error starting thread");
        return false;
    }

    return true;
}

bool DinRailControlBoardNWSYarp::detach()
{
    // Ensure that the device is not running
    if (isRunning())
    {
        stop();
    }

    closeDevice();

    return true;
}

void DinRailControlBoardNWSYarp::run()
{
    // check we are not overflowing with input messages
    constexpr int reads_for_warning = 20;
    if (inputStreamingPort.getPendingReads() >= reads_for_warning)
    {
        controlBoardLogger().warn("[{}] Number of streaming input messages to be read is {} and "
                                  "can overflow",
                                  id(),
                                  inputStreamingPort.getPendingReads());
    }
    // handle stateExt first
    ControlBoardYARPJointData& data = extendedOutputState_buffer.get();

    data.jointPosition.resize(subdevice_joints);
    data.jointVelocity.resize(subdevice_joints);
    data.jointAcceleration.resize(subdevice_joints);
    data.motorPosition.resize(subdevice_joints);
    data.motorVelocity.resize(subdevice_joints);
    data.motorAcceleration.resize(subdevice_joints);
    data.torque.resize(subdevice_joints);
    data.pwmDutycycle.resize(subdevice_joints);
    data.current.resize(subdevice_joints);
    data.controlMode.resize(subdevice_joints);
    data.interactionMode.resize(subdevice_joints);
    data.temperature.resize(subdevice_joints);

    // Get data from HW
    if (iEncodersTimed)
    {
        data.jointPosition_isValid
            = iEncodersTimed->getEncodersTimed(data.jointPosition.data(), times.data());
        data.jointVelocity_isValid = iEncodersTimed->getEncoderSpeeds(data.jointVelocity.data());
        data.jointAcceleration_isValid
            = iEncodersTimed->getEncoderAccelerations(data.jointAcceleration.data());
    } else
    {
        data.jointPosition_isValid = false;
        data.jointVelocity_isValid = false;
        data.jointAcceleration_isValid = false;
    }

    if (iMotorEncoders)
    {
        data.motorPosition_isValid = iMotorEncoders->getMotorEncoders(data.motorPosition.data());
        data.motorVelocity_isValid
            = iMotorEncoders->getMotorEncoderSpeeds(data.motorVelocity.data());
        data.motorAcceleration_isValid
            = iMotorEncoders->getMotorEncoderAccelerations(data.motorAcceleration.data());
    } else
    {
        data.motorPosition_isValid = false;
        data.motorVelocity_isValid = false;
        data.motorAcceleration_isValid = false;
    }

    if (iMotor)
    {
        data.temperature_isValid = iMotor->getTemperatures(data.temperature.data());
    } else
    {
        data.temperature_isValid = false;
    }

    if (iTorqueControl)
    {
        data.torque_isValid = iTorqueControl->getTorques(data.torque.data());
    } else
    {
        data.torque_isValid = false;
    }

    if (iPWMControl)
    {
        data.pwmDutycycle_isValid = iPWMControl->getDutyCycles(data.pwmDutycycle.data());
    } else
    {
        data.pwmDutycycle_isValid = false;
    }

    if (iCurrentControl)
    {
        data.current_isValid = iCurrentControl->getCurrents(data.current.data());
    } else if (iAmplifierControl)
    {
        data.current_isValid = iAmplifierControl->getCurrents(data.current.data());
    } else
    {
        data.current_isValid = false;
    }

    if (iControlMode)
    {
        data.controlMode_isValid = iControlMode->getControlModes(data.controlMode.data());
    } else
    {
        data.controlMode_isValid = false;
    }

    if (iInteractionMode)
    {
        data.interactionMode_isValid = iInteractionMode->getInteractionModes(
            reinterpret_cast<yarp::dev::InteractionModeEnum*>(data.interactionMode.data()));
    } else
    {
        data.interactionMode_isValid = false;
    }

    // Check if the encoders timestamps are consistent.
    for (double tt : times)
    {
        if (std::abs(times[0] - tt) > 1.0)
        {
            if (controlBoardShouldLogThrottle(id(), 1.0))
            {
                controlBoardLogger().error("[{}] Encoder timestamps are not consistent! Data will "
                                           "not be published.",
                                           id());
            }
            return;
        }
    }

    // Update the port envelope time by averaging all timestamps
    time.update(std::accumulate(times.begin(), times.end(), 0.0) / subdevice_joints);
    yarp::os::Stamp averageTime = time;

    extendedOutputStatePort.setEnvelope(averageTime);
    extendedOutputState_buffer.write();

    // handle state:o
    yarp::sig::Vector& v = outputPositionStatePort.prepare();
    v.resize(subdevice_joints);
    std::copy(data.jointPosition.begin(), data.jointPosition.end(), v.begin());

    outputPositionStatePort.setEnvelope(averageTime);
    outputPositionStatePort.write();
}
