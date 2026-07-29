/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "DinRailControlBoardRemapperHelpers.h"
#include "DinRailControlBoardRemapperLogComponent.h"

#include <spdlog/spdlog.h>

#include <cassert>

using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;

using dinrail::yarp_devices::remapper::controlBoardRemapperLogger;

RemappedSubControlBoard::RemappedSubControlBoard()
{
    id = "";

    pid = nullptr;
    pos = nullptr;
    posDir = nullptr;
    vel = nullptr;
    iJntEnc = nullptr;
    iMotEnc = nullptr;
    amp = nullptr;
    lim = nullptr;
    calib = nullptr;
    remcalib = nullptr;
    iTimed = nullptr;
    info = nullptr;
    iTorque = nullptr;
    iImpedance = nullptr;
    iImpedanceAllSetPointsControl = nullptr;
    iMode = nullptr;
    iInteract = nullptr;
    imotor = nullptr;
    iVar = nullptr;
    iPwm = nullptr;
    iCurr = nullptr;

    subdevice = nullptr;

    attachedF = false;
    _subDevVerbose = false;
}

void RemappedSubControlBoard::detach()
{
    subdevice = nullptr;

    pid = nullptr;
    pos = nullptr;
    posDir = nullptr;
    vel = nullptr;
    amp = nullptr;
    iJntEnc = nullptr;
    iMotEnc = nullptr;
    lim = nullptr;
    calib = nullptr;
    info = nullptr;
    iTorque = nullptr;
    iImpedance = nullptr;
    iImpedanceAllSetPointsControl = nullptr;
    iMode = nullptr;
    iTimed = nullptr;
    iInteract = nullptr;
    imotor = nullptr;
    iVar = nullptr;
    iPwm = nullptr;
    iCurr = nullptr;

    attachedF = false;
}

bool RemappedSubControlBoard::attach(yarp::dev::PolyDriver* d, const std::string& k)
{
    if (id != k)
    {
        controlBoardRemapperLogger().error("Wrong device {}", k.c_str());
        return false;
    }

    if (d == nullptr)
    {
        controlBoardRemapperLogger().error("Invalid device (null pointer)");
        return false;
    }

    subdevice = d;

    if (subdevice->isValid())
    {
        subdevice->view(pid);
        subdevice->view(pos);
        subdevice->view(posDir);
        subdevice->view(vel);
        subdevice->view(amp);
        subdevice->view(lim);
        subdevice->view(calib);
        subdevice->view(remcalib);
        subdevice->view(info);
        subdevice->view(iTimed);
        subdevice->view(iTorque);
        subdevice->view(iImpedance);
        subdevice->view(iImpedanceAllSetPointsControl);
        subdevice->view(iMode);
        subdevice->view(iJntEnc);
        subdevice->view(iMotEnc);
        subdevice->view(iInteract);
        subdevice->view(imotor);
        subdevice->view(iVar);
        subdevice->view(iPwm);
        subdevice->view(iCurr);
        subdevice->view(iFault);
    } else
    {
        controlBoardRemapperLogger().error("Invalid device {} (isValid() returned false)", k);
        return false;
    }

    if ((iTorque == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("iTorque not valid interface");
    }

    if ((iImpedance == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("iImpedance not valid interface");
    }

    if ((iInteract == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("iInteractionMode not valid interface");
    }

    if ((iMotEnc == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("IMotorEncoders not valid interface");
    }

    if ((imotor == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("IMotor not valid interface");
    }

    if ((iVar == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("IRemoveVariables not valid interface");
    }

    if ((info == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("IAxisInfo not valid interface");
    }

    if ((iPwm == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("IPWMControl not valid interface");
    }

    if ((iCurr == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("ICurrentControl not valid interface");
    }

    if ((iFault == nullptr) && (_subDevVerbose))
    {
        controlBoardRemapperLogger().warn("IJointFault not valid interface");
    }

    // checking minimum set of interfaces required
    if (!(pos))
    {
        controlBoardRemapperLogger().warn("IPositionControl interface was not found in subdevice");
    }

    if (!(vel))
    {
        controlBoardRemapperLogger().warn("IVelocityControl interface was not found in subdevice");
    }

    if (!iJntEnc)
    {
        controlBoardRemapperLogger().warn("IEncoderTimed interface was not found in subdevice");
    }

    if (!iMode)
    {
        controlBoardRemapperLogger().warn("IControlMode interface was not found in subdevice");
    }

    int deviceJoints = 0;
    if (pos != nullptr)
    {
        if (!pos->getAxes(&deviceJoints))
        {
            controlBoardRemapperLogger().error("failed to get axes number for subdevice {}",
                                               k.c_str());
            return false;
        }
        if (deviceJoints <= 0)
        {
            controlBoardRemapperLogger().error("attached device has an invalid number of joints "
                                               "({})",
                                               deviceJoints);
            return false;
        }
    } else if (info != nullptr)
    {
        if (!info->getAxes(&deviceJoints))
        {
            controlBoardRemapperLogger().error("failed to get axes number for subdevice {}",
                                               k.c_str());
            return false;
        }
        if (deviceJoints <= 0)
        {
            controlBoardRemapperLogger().error("attached device has an invalid number of joints "
                                               "({})",
                                               deviceJoints);
            return false;
        }
    } else
    {
        controlBoardRemapperLogger().error("attached device has no IPositionControl nor IAxisInfo "
                                           "interface");
        return false;
    }

    attachedF = true;
    return true;
}

bool ControlBoardSubControlBoardAxesDecomposition::configure(
    const RemappedControlBoards& remappedControlBoards)
{
    // Resize buffers
    m_nrOfControlledAxesInRemappedCtrlBrd = remappedControlBoards.getNrOfRemappedAxes();

    size_t nrOfSubControlBoards = remappedControlBoards.getNrOfSubControlBoards();

    m_nJointsInSubControlBoard.resize(nrOfSubControlBoards, 0);
    m_jointsInSubControlBoard.resize(nrOfSubControlBoards);

    m_bufferForSubControlBoard.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardControlModes.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardInteractionModes.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointPos.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointVel.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointTorque.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointStiffness.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointDamping.resize(nrOfSubControlBoards);

    m_counterForControlBoard.resize(nrOfSubControlBoards);

    for (size_t ctrlBrd = 0; ctrlBrd < nrOfSubControlBoards; ctrlBrd++)
    {
        m_nJointsInSubControlBoard[ctrlBrd] = 0;
        m_jointsInSubControlBoard[ctrlBrd].clear();
        m_bufferForSubControlBoard[ctrlBrd].clear();
        m_bufferForSubControlBoardControlModes[ctrlBrd].clear();
        m_bufferForSubControlBoardInteractionModes[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointPos[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointVel[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointTorque[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointStiffness[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointDamping[ctrlBrd].clear();
    }

    // Fill buffers
    for (size_t j = 0; j < remappedControlBoards.getNrOfRemappedAxes(); j++)
    {
        int off = (int)remappedControlBoards.lut[j].axisIndexInSubControlBoard;
        size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;

        m_nJointsInSubControlBoard[subIndex]++;
        m_jointsInSubControlBoard[subIndex].push_back(off);
    }

    // Allocate enough space in buffers
    for (size_t ctrlBrd = 0; ctrlBrd < nrOfSubControlBoards; ctrlBrd++)
    {
        m_bufferForSubControlBoard[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardControlModes[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardInteractionModes[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointPos[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointVel[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointTorque[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointStiffness[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointDamping[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);

        m_counterForControlBoard[ctrlBrd] = 0;
    }

    return true;
}

void ControlBoardSubControlBoardAxesDecomposition::fillSubControlBoardBuffersFromCompleteJointVector(
    const double* full, const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_bufferForSubControlBoard[ctrlBrd].clear();
    }

    for (int j = 0; j < m_nrOfControlledAxesInRemappedCtrlBrd; j++)
    {
        size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;

        m_bufferForSubControlBoard[subIndex].push_back(full[j]);
    }
}

void ControlBoardSubControlBoardAxesDecomposition::fillCompleteJointVectorFromSubControlBoardBuffers(
    double* full, const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_counterForControlBoard[ctrlBrd] = 0;
    }

    for (int j = 0; j < m_nrOfControlledAxesInRemappedCtrlBrd; j++)
    {
        size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;
        full[j] = m_bufferForSubControlBoard[subIndex][m_counterForControlBoard[subIndex]];
        m_counterForControlBoard[subIndex]++;
    }
}

void ControlBoardSubControlBoardAxesDecomposition::fillSubControlBoardBuffersFromCompleteJointVector(
    const int* full, const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_bufferForSubControlBoardControlModes[ctrlBrd].clear();
    }

    for (int j = 0; j < m_nrOfControlledAxesInRemappedCtrlBrd; j++)
    {
        size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;

        m_bufferForSubControlBoardControlModes[subIndex].push_back(full[j]);
    }
}

void ControlBoardSubControlBoardAxesDecomposition::fillCompleteJointVectorFromSubControlBoardBuffers(
    int* full, const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_counterForControlBoard[ctrlBrd] = 0;
    }

    for (int j = 0; j < m_nrOfControlledAxesInRemappedCtrlBrd; j++)
    {
        size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;
        full[j]
            = m_bufferForSubControlBoardControlModes[subIndex][m_counterForControlBoard[subIndex]];
        m_counterForControlBoard[subIndex]++;
    }
}

void ControlBoardSubControlBoardAxesDecomposition::fillSubControlBoardBuffersFromCompleteJointVector(
    const InteractionModeEnum* full, const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_bufferForSubControlBoardInteractionModes[ctrlBrd].clear();
    }

    for (int j = 0; j < m_nrOfControlledAxesInRemappedCtrlBrd; j++)
    {
        size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;

        m_bufferForSubControlBoardInteractionModes[subIndex].push_back(full[j]);
    }
}

void ControlBoardSubControlBoardAxesDecomposition::fillCompleteJointVectorFromSubControlBoardBuffers(
    InteractionModeEnum* full, const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_counterForControlBoard[ctrlBrd] = 0;
    }

    for (int j = 0; j < m_nrOfControlledAxesInRemappedCtrlBrd; j++)
    {
        size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;
        full[j] = m_bufferForSubControlBoardInteractionModes[subIndex]
                                                            [m_counterForControlBoard[subIndex]];
        m_counterForControlBoard[subIndex]++;
    }
}

void ControlBoardSubControlBoardAxesDecomposition::
    fillSubControlBoardSetPointBuffersFromCompleteJointVectors(
        const double* pos,
        const double* vel,
        const double* torque,
        const double* stiffness,
        const double* damping,
        const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_counterForControlBoard[ctrlBrd] = 0;
    }

    for (int j = 0; j < m_nrOfControlledAxesInRemappedCtrlBrd; j++)
    {
        const size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;
        const int counter = m_counterForControlBoard[subIndex]++;

        m_bufferForSubControlBoardSetPointPos[subIndex][counter] = pos[j];
        m_bufferForSubControlBoardSetPointVel[subIndex][counter] = vel[j];
        m_bufferForSubControlBoardSetPointTorque[subIndex][counter] = torque[j];
        m_bufferForSubControlBoardSetPointStiffness[subIndex][counter] = stiffness[j];
        m_bufferForSubControlBoardSetPointDamping[subIndex][counter] = damping[j];
    }
}

void ControlBoardSubControlBoardAxesDecomposition::
    fillCompleteJointVectorsFromSubControlBoardSetPointBuffers(
        double* pos,
        double* vel,
        double* torque,
        double* stiffness,
        double* damping,
        const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_counterForControlBoard[ctrlBrd] = 0;
    }

    for (int j = 0; j < m_nrOfControlledAxesInRemappedCtrlBrd; j++)
    {
        const size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;
        const int counter = m_counterForControlBoard[subIndex]++;

        pos[j] = m_bufferForSubControlBoardSetPointPos[subIndex][counter];
        vel[j] = m_bufferForSubControlBoardSetPointVel[subIndex][counter];
        torque[j] = m_bufferForSubControlBoardSetPointTorque[subIndex][counter];
        stiffness[j] = m_bufferForSubControlBoardSetPointStiffness[subIndex][counter];
        damping[j] = m_bufferForSubControlBoardSetPointDamping[subIndex][counter];
    }
}

bool ControlBoardArbitraryAxesDecomposition::configure(
    const RemappedControlBoards& remappedControlBoards)
{
    // Resize buffers
    size_t nrOfSubControlBoards = remappedControlBoards.getNrOfSubControlBoards();

    m_nJointsInSubControlBoard.resize(nrOfSubControlBoards, 0);
    m_jointsInSubControlBoard.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoard.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardControlModes.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardInteractionModes.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointPos.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointVel.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointTorque.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointStiffness.resize(nrOfSubControlBoards);
    m_bufferForSubControlBoardSetPointDamping.resize(nrOfSubControlBoards);

    m_counterForControlBoard.resize(nrOfSubControlBoards);

    for (size_t ctrlBrd = 0; ctrlBrd < nrOfSubControlBoards; ctrlBrd++)
    {
        m_jointsInSubControlBoard[ctrlBrd].clear();
        m_bufferForSubControlBoard[ctrlBrd].clear();
        m_bufferForSubControlBoardControlModes[ctrlBrd].clear();
        m_bufferForSubControlBoardInteractionModes[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointPos[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointVel[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointTorque[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointStiffness[ctrlBrd].clear();
        m_bufferForSubControlBoardSetPointDamping[ctrlBrd].clear();
    }

    // Count the maximum number of joints
    for (size_t j = 0; j < remappedControlBoards.getNrOfRemappedAxes(); j++)
    {
        size_t subIndex = remappedControlBoards.lut[j].subControlBoardIndex;

        m_nJointsInSubControlBoard[subIndex]++;
    }

    // Allocate enough space in buffers
    for (size_t ctrlBrd = 0; ctrlBrd < nrOfSubControlBoards; ctrlBrd++)
    {
        m_bufferForSubControlBoard[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardControlModes[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardInteractionModes[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointPos[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointVel[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointTorque[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointStiffness[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointDamping[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);

        m_counterForControlBoard[ctrlBrd] = 0;
        m_jointsInSubControlBoard[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
    }

    return true;
}

void ControlBoardArbitraryAxesDecomposition::fillArbitraryJointVectorFromSubControlBoardBuffers(
    double* arbitraryVec,
    const int n_joints,
    const int* joints,
    const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_counterForControlBoard[ctrlBrd] = 0;
    }

    for (int j = 0; j < n_joints; j++)
    {
        size_t subIndex = remappedControlBoards.lut[joints[j]].subControlBoardIndex;
        arbitraryVec[j] = m_bufferForSubControlBoard[subIndex][m_counterForControlBoard[subIndex]];
        m_counterForControlBoard[subIndex]++;
    }
}

void ControlBoardArbitraryAxesDecomposition::fillSubControlBoardBuffersFromArbitraryJointVector(
    const double* arbitraryVec,
    const int n_joints,
    const int* joints,
    const RemappedControlBoards& remappedControlBoards)
{
    this->createListOfJointsDecomposition(n_joints, joints, remappedControlBoards);

    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_bufferForSubControlBoard[ctrlBrd].clear();
    }

    for (int j = 0; j < n_joints; j++)
    {
        size_t subIndex = remappedControlBoards.lut[joints[j]].subControlBoardIndex;
        m_bufferForSubControlBoard[subIndex].push_back(arbitraryVec[j]);
    }
}

void ControlBoardArbitraryAxesDecomposition::fillArbitraryJointVectorFromSubControlBoardBuffers(
    int* arbitraryVec,
    const int n_joints,
    const int* joints,
    const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_counterForControlBoard[ctrlBrd] = 0;
    }

    for (int j = 0; j < n_joints; j++)
    {
        size_t subIndex = remappedControlBoards.lut[joints[j]].subControlBoardIndex;
        arbitraryVec[j]
            = m_bufferForSubControlBoardControlModes[subIndex][m_counterForControlBoard[subIndex]];
        m_counterForControlBoard[subIndex]++;
    }
}

void ControlBoardArbitraryAxesDecomposition::fillSubControlBoardBuffersFromArbitraryJointVector(
    const int* arbitraryVec,
    const int n_joints,
    const int* joints,
    const RemappedControlBoards& remappedControlBoards)
{
    this->createListOfJointsDecomposition(n_joints, joints, remappedControlBoards);

    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_bufferForSubControlBoardControlModes[ctrlBrd].clear();
    }

    for (int j = 0; j < n_joints; j++)
    {
        size_t subIndex = remappedControlBoards.lut[joints[j]].subControlBoardIndex;
        m_bufferForSubControlBoardControlModes[subIndex].push_back(arbitraryVec[j]);
    }
}

void ControlBoardArbitraryAxesDecomposition::fillArbitraryJointVectorFromSubControlBoardBuffers(
    InteractionModeEnum* arbitraryVec,
    const int n_joints,
    const int* joints,
    const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_counterForControlBoard[ctrlBrd] = 0;
    }

    for (int j = 0; j < n_joints; j++)
    {
        size_t subIndex = remappedControlBoards.lut[joints[j]].subControlBoardIndex;
        arbitraryVec[j]
            = m_bufferForSubControlBoardInteractionModes[subIndex]
                                                        [m_counterForControlBoard[subIndex]];
        m_counterForControlBoard[subIndex]++;
    }
}

void ControlBoardArbitraryAxesDecomposition::fillSubControlBoardBuffersFromArbitraryJointVector(
    const InteractionModeEnum* arbitraryVec,
    const int n_joints,
    const int* joints,
    const RemappedControlBoards& remappedControlBoards)
{
    this->createListOfJointsDecomposition(n_joints, joints, remappedControlBoards);

    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_bufferForSubControlBoardInteractionModes[ctrlBrd].clear();
    }

    for (int j = 0; j < n_joints; j++)
    {
        size_t subIndex = remappedControlBoards.lut[joints[j]].subControlBoardIndex;
        m_bufferForSubControlBoardInteractionModes[subIndex].push_back(arbitraryVec[j]);
    }
}

void ControlBoardArbitraryAxesDecomposition::createListOfJointsDecomposition(
    const int n_joints, const int* joints, const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_nJointsInSubControlBoard[ctrlBrd] = 0;
        m_jointsInSubControlBoard[ctrlBrd].clear();
    }

    // Fill buffers
    for (int j = 0; j < n_joints; j++)
    {
        int off = (int)remappedControlBoards.lut[joints[j]].axisIndexInSubControlBoard;
        size_t subIndex = remappedControlBoards.lut[joints[j]].subControlBoardIndex;

        m_nJointsInSubControlBoard[subIndex]++;
        m_jointsInSubControlBoard[subIndex].push_back(off);
    }
}

void ControlBoardArbitraryAxesDecomposition::resizeSubControlBoardBuffers(
    const int n_joints, const int* joints, const RemappedControlBoards& remappedControlBoards)
{
    // Properly populate the m_nJointsInSubControlBoard and m_jointsInSubControlBoard methods
    createListOfJointsDecomposition(n_joints, joints, remappedControlBoards);

    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        const bool jointCountMatches = static_cast<size_t>(m_nJointsInSubControlBoard[ctrlBrd])
                                       == m_jointsInSubControlBoard[ctrlBrd].size();
        if (!jointCountMatches)
        {
            controlBoardRemapperLogger().error("Assertion failed: "
                                               "static_cast<size_t>(m_nJointsInSubControlBoard["
                                               "ctrlBrd]) == "
                                               "m_jointsInSubControlBoard[ctrlBrd].size()");
            assert(jointCountMatches);
        }
        m_bufferForSubControlBoard[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
    }
}

void ControlBoardArbitraryAxesDecomposition::
    fillSubControlBoardSetPointBuffersFromArbitraryJointVectors(
        const double* pos,
        const double* vel,
        const double* torque,
        const double* stiffness,
        const double* damping,
        const int n_joints,
        const int* joints,
        const RemappedControlBoards& remappedControlBoards)
{
    this->createListOfJointsDecomposition(n_joints, joints, remappedControlBoards);

    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        const bool jointCountMatches = static_cast<size_t>(m_nJointsInSubControlBoard[ctrlBrd])
                                       == m_jointsInSubControlBoard[ctrlBrd].size();
        if (!jointCountMatches)
        {
            controlBoardRemapperLogger().error("Assertion failed: "
                                               "static_cast<size_t>(m_nJointsInSubControlBoard["
                                               "ctrlBrd]) == "
                                               "m_jointsInSubControlBoard[ctrlBrd].size()");
            assert(jointCountMatches);
        }

        m_counterForControlBoard[ctrlBrd] = 0;
        m_bufferForSubControlBoardSetPointPos[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointVel[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointTorque[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointStiffness[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointDamping[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
    }

    for (int j = 0; j < n_joints; j++)
    {
        const size_t subIndex = remappedControlBoards.lut[joints[j]].subControlBoardIndex;
        const int counter = m_counterForControlBoard[subIndex]++;

        m_bufferForSubControlBoardSetPointPos[subIndex][counter] = pos[j];
        m_bufferForSubControlBoardSetPointVel[subIndex][counter] = vel[j];
        m_bufferForSubControlBoardSetPointTorque[subIndex][counter] = torque[j];
        m_bufferForSubControlBoardSetPointStiffness[subIndex][counter] = stiffness[j];
        m_bufferForSubControlBoardSetPointDamping[subIndex][counter] = damping[j];
    }
}

void ControlBoardArbitraryAxesDecomposition::resizeSubControlBoardSetPointBuffers(
    const int n_joints, const int* joints, const RemappedControlBoards& remappedControlBoards)
{
    this->createListOfJointsDecomposition(n_joints, joints, remappedControlBoards);

    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        const bool jointCountMatches = static_cast<size_t>(m_nJointsInSubControlBoard[ctrlBrd])
                                       == m_jointsInSubControlBoard[ctrlBrd].size();
        if (!jointCountMatches)
        {
            controlBoardRemapperLogger().error("Assertion failed: "
                                               "static_cast<size_t>(m_nJointsInSubControlBoard["
                                               "ctrlBrd]) == "
                                               "m_jointsInSubControlBoard[ctrlBrd].size()");
            assert(jointCountMatches);
        }

        m_bufferForSubControlBoardSetPointPos[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointVel[ctrlBrd].resize(m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointTorque[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointStiffness[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
        m_bufferForSubControlBoardSetPointDamping[ctrlBrd].resize(
            m_nJointsInSubControlBoard[ctrlBrd]);
    }
}

void ControlBoardArbitraryAxesDecomposition::
    fillArbitraryJointVectorsFromSubControlBoardSetPointBuffers(
        double* pos,
        double* vel,
        double* torque,
        double* stiffness,
        double* damping,
        const int n_joints,
        const int* joints,
        const RemappedControlBoards& remappedControlBoards)
{
    for (size_t ctrlBrd = 0; ctrlBrd < remappedControlBoards.getNrOfSubControlBoards(); ctrlBrd++)
    {
        m_counterForControlBoard[ctrlBrd] = 0;
    }

    for (int j = 0; j < n_joints; j++)
    {
        const size_t subIndex = remappedControlBoards.lut[joints[j]].subControlBoardIndex;
        const int counter = m_counterForControlBoard[subIndex]++;

        pos[j] = m_bufferForSubControlBoardSetPointPos[subIndex][counter];
        vel[j] = m_bufferForSubControlBoardSetPointVel[subIndex][counter];
        torque[j] = m_bufferForSubControlBoardSetPointTorque[subIndex][counter];
        stiffness[j] = m_bufferForSubControlBoardSetPointStiffness[subIndex][counter];
        damping[j] = m_bufferForSubControlBoardSetPointDamping[subIndex][counter];
    }
}
