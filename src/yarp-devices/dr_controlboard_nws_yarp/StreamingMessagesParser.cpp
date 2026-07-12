/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-FileCopyrightText: 2006-2010 RobotCub Consortium
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "StreamingMessagesParser.h"

#include <yarp/os/LogStream.h>

#include "ControlBoardLogComponent.h"
#include <dinrail/ControlBoardYARPProtocolSharedDefinitions.h>
#include <iostream>

using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::dev::impl;
using namespace yarp::sig;


void StreamingMessagesParser::init(yarp::dev::DeviceDriver* x)
{
    stream_nJoints = 0;
    x->view(stream_IPosCtrl);
    x->view(stream_IPosDirect);
    x->view(stream_IVel);
    x->view(stream_ITorque);
    x->view(stream_IImpedance);
    x->view(stream_IImpedanceAllSetPointsControl);
    x->view(stream_IPWM);
    x->view(stream_ICurrent);
    x->view(stream_IAxis);
}

void StreamingMessagesParser::reset()
{
    stream_nJoints = 0;
    stream_IPosCtrl = nullptr;
    stream_IPosDirect = nullptr;
    stream_IVel = nullptr;
    stream_ITorque = nullptr;
    stream_IImpedance = nullptr;
    stream_IImpedanceAllSetPointsControl = nullptr;
    stream_IPWM = nullptr;
    stream_ICurrent = nullptr;
    stream_IAxis = nullptr;
}

bool StreamingMessagesParser::initialize()
{
    stream_nJoints = 0;
    if (stream_IPosCtrl) { stream_IPosCtrl->getAxes(&stream_nJoints); return true; }
    if (stream_IVel)     { stream_IVel->getAxes(&stream_nJoints); return true; }
    if (stream_ITorque)  { stream_ITorque->getAxes(&stream_nJoints); return true; }
    if (stream_IAxis)    { stream_IAxis->getAxes(&stream_nJoints); return true; }

    yCError(CONTROLBOARD, "Unable to get number of joints");
    return false;
}

// streaming port callback
void StreamingMessagesParser::onRead(CommandMessage& v)
{
    Bottle& b = v.head;
    Vector& cmdVector = v.body;

    //Use the following only for debug, since it can heavily slow down the system
    yCTrace(CONTROLBOARD, "Received command %s, %s\n", b.toString().c_str(), cmdVector.toString().c_str());

    // some consistency checks
    size_t maxCommandVectorSize = static_cast<size_t>(stream_nJoints);
    if (b.get(0).asVocab32() == dinrail::VOCAB_DINRAIL_IMPEDANCE_ALL_SETPOINTS) {
        maxCommandVectorSize = 5 * static_cast<size_t>(stream_nJoints);
    }

    if (cmdVector.size() > maxCommandVectorSize) {
        std::string str = yarp::os::Vocab32::decode(b.get(0).asVocab32());
        yCError(CONTROLBOARD, "Received command vector with number of elements bigger than axis controlled by this wrapper (cmd: %s requested jnts: %zu received jnts: %zu)\n", str.c_str(), maxCommandVectorSize, cmdVector.size());
        return;
    }
    if (cmdVector.data() == nullptr) {
        yCError(CONTROLBOARD, "Received null command vector");
        return;
    }

    switch (b.get(0).asVocab32()) {
    // manage commands with interface name as first
    case VOCAB_PWMCONTROL_INTERFACE: {
        switch (b.get(1).asVocab32()) {
        case VOCAB_PWMCONTROL_REF_PWM: {
            if (stream_IPWM) {
                bool ok = stream_IPWM->setRefDutyCycle(b.get(2).asInt32(), cmdVector[0]);
                if (!ok) {
                    yCError(CONTROLBOARD, "Errors while trying to command an pwm message");
                }
            } else {
                yCError(CONTROLBOARD, "PWM interface not valid");
            }
        } break;
        case VOCAB_PWMCONTROL_REF_PWMS: {
            if (stream_IPWM) {
                bool ok = stream_IPWM->setRefDutyCycles(cmdVector.data());
                if (!ok) {
                    yCError(CONTROLBOARD, "Errors while trying to command an pwm message");
                }
            } else {
                yCError(CONTROLBOARD, "PWM interface not valid");
            }
        } break;
        }
    } break;

    case VOCAB_CURRENTCONTROL_INTERFACE: {
        switch (b.get(1).asVocab32()) {
        case VOCAB_CURRENT_REF: {
            if (stream_ICurrent) {
                bool ok = stream_ICurrent->setRefCurrent(b.get(2).asInt32(), cmdVector[0]);
                if (!ok) {
                    yCError(CONTROLBOARD, "Errors while trying to command a streaming current message on single joint\n");
                }
            }
        } break;
        case VOCAB_CURRENT_REFS: {
            if (stream_ICurrent) {
                bool ok = stream_ICurrent->setRefCurrents(cmdVector.data());
                if (!ok) {
                    yCError(CONTROLBOARD, "Errors while trying to command a streaming current message on all joints\n");
                }
            }
        } break;
        case VOCAB_CURRENT_REF_GROUP: {
            if (stream_ICurrent) {
                int n_joints = b.get(2).asInt32();
                Bottle* jlut = b.get(3).asList();
                if ((static_cast<int>(jlut->size()) != n_joints) && (static_cast<int>(cmdVector.size()) != n_joints)) {
                    yCError(CONTROLBOARD, "Received VOCAB_CURRENT_REF_GROUP size of joints vector or currents vector does not match the selected joint number\n");
                }

                int* joint_list = new int[n_joints];
                for (int i = 0; i < n_joints; i++) {
                    joint_list[i] = jlut->get(i).asInt32();
                }


                bool ok = stream_ICurrent->setRefCurrents(n_joints, joint_list, cmdVector.data());
                if (!ok) {
                    yCError(CONTROLBOARD, "Error while trying to command a streaming current message on joint group\n");
                }

                delete[] joint_list;
            }
        } break;
        default:
        {
            std::string str = yarp::os::Vocab32::decode(b.get(0).asVocab32());
            yCError(CONTROLBOARD, "Unrecognized message while receiving on command port (%s)\n", str.c_str());
        } break;
        }
    } break;

    // fallback to commands without interface name
    case VOCAB_POSITION_MODE: {
        yCError(CONTROLBOARD, "Received VOCAB_POSITION_MODE this is an send invalid message on streaming port");
        break;
    }

    case VOCAB_POSITION_MOVES: {
        if (stream_IPosCtrl) {
            bool ok = stream_IPosCtrl->positionMove(cmdVector.data());
            if (!ok) {
                yCError(CONTROLBOARD, "Errors while trying to start a position move");
            }
        }

    } break;

    case VOCAB_VELOCITY_MODE: {
        yCError(CONTROLBOARD, "Received VOCAB_VELOCITY_MODE this is an send invalid message on streaming port");
        break;
    }

    case VOCAB_VELOCITY_MOVE: {
        if (stream_IVel) {
           bool ok = stream_IVel->velocityMove(b.get(1).asInt32(), cmdVector[0]);
           if (!ok) {
               yCError(CONTROLBOARD, "Errors while trying to start a velocity move");
           }
        }
    } break;

    case VOCAB_VELOCITY_MOVES: {
        if (stream_IVel) {
            bool ok = stream_IVel->velocityMove(cmdVector.data());
            if (!ok) {
                yCError(CONTROLBOARD, "Errors while trying to start a velocity move");
            }
        }
    } break;

    case VOCAB_POSITION_DIRECT: {
        if (stream_IPosDirect) {
            bool ok = stream_IPosDirect->setPosition(b.get(1).asInt32(), cmdVector[0]); // cmdVector.data());
            if (!ok) {
                yCError(CONTROLBOARD, "Errors while trying to command an streaming position direct message on joint %d\n", b.get(1).asInt32());
            }
        }
    } break;

    case VOCAB_TORQUES_DIRECT: {
        if (stream_ITorque) {
            bool ok = stream_ITorque->setRefTorque(b.get(1).asInt32(), cmdVector[0]);
            if (!ok) {
                yCError(CONTROLBOARD, "Errors while trying to command a streaming torque direct message on single joint\n");
            }
        }
    } break;

    case VOCAB_TORQUES_DIRECTS: {
        if (stream_ITorque) {
            bool ok = stream_ITorque->setRefTorques(cmdVector.data());
            if (!ok) {
                yCError(CONTROLBOARD, "Errors while trying to command a streaming torque direct message on all joints\n");
            }
        }
    } break;

    case VOCAB_TORQUES_DIRECT_GROUP: {
        if (stream_ITorque) {
            int n_joints = b.get(1).asInt32();
            Bottle* jlut = b.get(2).asList();
            if ((static_cast<int>(jlut->size()) != n_joints) && (static_cast<int>(cmdVector.size()) != n_joints)) {
                yCError(CONTROLBOARD, "Received VOCAB_TORQUES_DIRECT_GROUP size of joints vector or torques vector does not match the selected joint number\n");
            }

            int* joint_list = new int[n_joints];
            for (int i = 0; i < n_joints; i++) {
                joint_list[i] = jlut->get(i).asInt32();
            }


            bool ok = stream_ITorque->setRefTorques(n_joints, joint_list, cmdVector.data());
            if (!ok) {
                yCError(CONTROLBOARD, "Error while trying to command a streaming toruqe direct message on joint group\n");
            }

            delete[] joint_list;
        }
    } break;

    case dinrail::VOCAB_DINRAIL_IMPEDANCE_ALL_SETPOINTS: {
        if (b.get(1).asVocab32() != dinrail::VOCAB_DINRAIL_SETPOINT) {
            yCError(CONTROLBOARD, "Received unsupported dinrail impedance-all-setpoints method on streaming port");
            break;
        }

        if (b.size() == 3) {
            if (cmdVector.size() != 5) {
                yCError(CONTROLBOARD, "Received invalid single-joint impedance-all-setpoints payload size %zu", cmdVector.size());
                break;
            }

            const int joint = b.get(2).asInt32();
            bool ok = false;
            if (stream_IImpedanceAllSetPointsControl) {
                ok = stream_IImpedanceAllSetPointsControl->setSetPoint(joint,
                                                                       cmdVector[0],
                                                                       cmdVector[1],
                                                                       cmdVector[2],
                                                                       cmdVector[3],
                                                                       cmdVector[4]);
            } else if (stream_emulateImpedanceAllSetPointsControl
                       && stream_IPosDirect
                       && stream_IVel
                       && stream_ITorque
                       && stream_IImpedance) {
                const bool setPosOk = stream_IPosDirect->setPosition(joint, cmdVector[0]);
                const bool setVelOk = stream_IVel->velocityMove(joint, cmdVector[1]);
                const bool setTorqueOk = stream_ITorque->setRefTorque(joint, cmdVector[2]);
                const bool setImpOk = stream_IImpedance->setImpedance(joint, cmdVector[3], cmdVector[4]);
                ok = setPosOk && setVelOk && setTorqueOk && setImpOk;
            }
            if (!ok) {
                yCError(CONTROLBOARD, "Error while trying to command single-joint impedance-all-setpoints message\n");
            }
            break;
        }

        if (b.size() == 4) {
            const int n_joints = b.get(2).asInt32();
            Bottle* jlut = b.get(3).asList();
            if (jlut == nullptr || static_cast<int>(jlut->size()) != n_joints || static_cast<int>(cmdVector.size()) != 5 * n_joints) {
                yCError(CONTROLBOARD, "Received invalid impedance-all-setpoints group payload\n");
                break;
            }

            std::vector<int> jointList(static_cast<size_t>(n_joints));
            for (int i = 0; i < n_joints; i++) {
                jointList[static_cast<size_t>(i)] = jlut->get(i).asInt32();
            }

            const double* raw = cmdVector.data();
            dinrail::VectorProxy<const int>::Ref jointsRef(jointList);
            dinrail::VectorProxy<const double>::Ref posRef(std::span<const double>(raw, static_cast<size_t>(n_joints)));
            dinrail::VectorProxy<const double>::Ref velRef(std::span<const double>(raw + n_joints, static_cast<size_t>(n_joints)));
            dinrail::VectorProxy<const double>::Ref torqueRef(std::span<const double>(raw + 2 * n_joints, static_cast<size_t>(n_joints)));
            dinrail::VectorProxy<const double>::Ref stiffnessRef(std::span<const double>(raw + 3 * n_joints, static_cast<size_t>(n_joints)));
            dinrail::VectorProxy<const double>::Ref dampingRef(std::span<const double>(raw + 4 * n_joints, static_cast<size_t>(n_joints)));

            bool ok = false;
            if (stream_IImpedanceAllSetPointsControl) {
                ok = stream_IImpedanceAllSetPointsControl->setSetPoints(jointsRef,
                                                                        posRef,
                                                                        velRef,
                                                                        torqueRef,
                                                                        stiffnessRef,
                                                                        dampingRef);
            } else if (stream_emulateImpedanceAllSetPointsControl
                       && stream_IPosDirect
                       && stream_IVel
                       && stream_ITorque
                       && stream_IImpedance) {
                bool setPosOk = true;
                bool setVelOk = true;
                bool setTorqueOk = true;
                bool setImpOk = true;
                for (int i = 0; i < n_joints; i++) {
                    const int joint = jointList[static_cast<size_t>(i)];
                    setPosOk = setPosOk && stream_IPosDirect->setPosition(joint, posRef[static_cast<size_t>(i)]);
                    setVelOk = setVelOk && stream_IVel->velocityMove(joint, velRef[static_cast<size_t>(i)]);
                    setTorqueOk = setTorqueOk && stream_ITorque->setRefTorque(joint, torqueRef[static_cast<size_t>(i)]);
                    setImpOk = setImpOk && stream_IImpedance->setImpedance(joint,
                                                                           stiffnessRef[static_cast<size_t>(i)],
                                                                           dampingRef[static_cast<size_t>(i)]);
                }
                ok = setPosOk && setVelOk && setTorqueOk && setImpOk;
            }
            if (!ok) {
                yCError(CONTROLBOARD, "Error while trying to command group impedance-all-setpoints message\n");
            }
            break;
        }

        if (b.size() == 2) {
            if (stream_nJoints <= 0 || static_cast<int>(cmdVector.size()) != 5 * stream_nJoints) {
                yCError(CONTROLBOARD, "Received invalid full-joint impedance-all-setpoints payload size %zu", cmdVector.size());
                break;
            }

            const double* raw = cmdVector.data();
            dinrail::VectorProxy<const double>::Ref posRef(std::span<const double>(raw, static_cast<size_t>(stream_nJoints)));
            dinrail::VectorProxy<const double>::Ref velRef(std::span<const double>(raw + stream_nJoints, static_cast<size_t>(stream_nJoints)));
            dinrail::VectorProxy<const double>::Ref torqueRef(std::span<const double>(raw + 2 * stream_nJoints, static_cast<size_t>(stream_nJoints)));
            dinrail::VectorProxy<const double>::Ref stiffnessRef(std::span<const double>(raw + 3 * stream_nJoints, static_cast<size_t>(stream_nJoints)));
            dinrail::VectorProxy<const double>::Ref dampingRef(std::span<const double>(raw + 4 * stream_nJoints, static_cast<size_t>(stream_nJoints)));

            bool ok = false;
            if (stream_IImpedanceAllSetPointsControl) {
                ok = stream_IImpedanceAllSetPointsControl->setSetPoints(posRef,
                                                                        velRef,
                                                                        torqueRef,
                                                                        stiffnessRef,
                                                                        dampingRef);
            } else if (stream_emulateImpedanceAllSetPointsControl
                       && stream_IPosDirect
                       && stream_IVel
                       && stream_ITorque
                       && stream_IImpedance) {
                bool setPosOk = true;
                bool setVelOk = true;
                bool setTorqueOk = true;
                bool setImpOk = true;
                for (int i = 0; i < stream_nJoints; i++) {
                    setPosOk = setPosOk && stream_IPosDirect->setPosition(i, posRef[static_cast<size_t>(i)]);
                    setVelOk = setVelOk && stream_IVel->velocityMove(i, velRef[static_cast<size_t>(i)]);
                    setTorqueOk = setTorqueOk && stream_ITorque->setRefTorque(i, torqueRef[static_cast<size_t>(i)]);
                    setImpOk = setImpOk && stream_IImpedance->setImpedance(i,
                                                                           stiffnessRef[static_cast<size_t>(i)],
                                                                           dampingRef[static_cast<size_t>(i)]);
                }
                ok = setPosOk && setVelOk && setTorqueOk && setImpOk;
            }
            if (!ok) {
                yCError(CONTROLBOARD, "Error while trying to command full-joint impedance-all-setpoints message\n");
            }
            break;
        }

        yCError(CONTROLBOARD, "Received malformed dinrail impedance-all-setpoints command header");
    } break;

    case VOCAB_POSITION_DIRECT_GROUP: {
        if (stream_IPosDirect) {
            int n_joints = b.get(1).asInt32();
            Bottle* jlut = b.get(2).asList();
            if ((static_cast<int>(jlut->size()) != n_joints) && (static_cast<int>(cmdVector.size()) != n_joints)) {
                yCError(CONTROLBOARD, "Received VOCAB_POSITION_DIRECT_GROUP size of joints vector or positions vector does not match the selected joint number\n");
            }

            int* joint_list = new int[n_joints];
            for (int i = 0; i < n_joints; i++) {
                joint_list[i] = jlut->get(i).asInt32();
            }


            bool ok = stream_IPosDirect->setPositions(n_joints, joint_list, cmdVector.data());
            if (!ok) {
                yCError(CONTROLBOARD, "Error while trying to command a streaming position direct message on joint group\n");
            }

            delete[] joint_list;
        }
    } break;

    case VOCAB_POSITION_DIRECTS: {
        if (stream_IPosDirect) {
            bool ok = stream_IPosDirect->setPositions(cmdVector.data());
            if (!ok) {
                yCError(CONTROLBOARD, "Error while trying to command a streaming position direct message on all joints\n");
            }
        }
    } break;

    case VOCAB_VELOCITY_MOVE_GROUP: {
        if (stream_IVel) {
            int n_joints = b.get(1).asInt32();
            Bottle* jlut = b.get(2).asList();
            if ((static_cast<int>(jlut->size()) != n_joints) && (static_cast<int>(cmdVector.size()) != n_joints)) {
                yCError(CONTROLBOARD, "Received VOCAB_VELOCITY_MOVE_GROUP size of joints vector or positions vector does not match the selected joint number\n");
            }

            int* joint_list = new int[n_joints];
            for (int i = 0; i < n_joints; i++) {
                joint_list[i] = jlut->get(i).asInt32();
            }

            bool ok = stream_IVel->velocityMove(n_joints, joint_list, cmdVector.data());
            if (!ok) {
                yCError(CONTROLBOARD, "Error while trying to command a velocity move on joint group\n");
            }

            delete[] joint_list;
        }
    } break;

    default:
    {
        std::string str = yarp::os::Vocab32::decode(b.get(0).asVocab32());
        yCError(CONTROLBOARD, "Unrecognized message while receiving on command port (%s)\n", str.c_str());
    } break;
    }
}
