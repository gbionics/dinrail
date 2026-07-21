/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DINRAIL_CONTROLBOARD_NWC_RTSHM_H
#define DINRAIL_CONTROLBOARD_NWC_RTSHM_H

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include <yarp/sig/Vector.h>

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/IAmplifierControl.h>
#include <yarp/dev/IAxisInfo.h>
#include <yarp/dev/IControlCalibration.h>
#include <yarp/dev/IControlLimits.h>
#include <yarp/dev/IControlMode.h>
#include <yarp/dev/ICurrentControl.h>
#include <yarp/dev/IEncodersTimed.h>
#include <yarp/dev/IImpedanceControl.h>
#include <yarp/dev/IInteractionMode.h>
#include <yarp/dev/IJointFault.h>
#include <yarp/dev/IMotor.h>
#include <yarp/dev/IMotorEncoders.h>
#include <yarp/dev/IPWMControl.h>
#include <yarp/dev/IPidControl.h>
#include <yarp/dev/IPositionControl.h>
#include <yarp/dev/IPositionDirect.h>
#include <yarp/dev/IPreciselyTimed.h>
#include <yarp/dev/IRemoteCalibrator.h>
#include <yarp/dev/IRemoteVariables.h>
#include <yarp/dev/ITorqueControl.h>
#include <yarp/dev/IVelocityControl.h>

#include <dinrail/IImpedanceAllSetPointsControl.h>

#include "DinRailControlBoardNWCRtShm_ParamsParser.h"

#include <RtShmLayout.h>
#include <RtShmProtocol.h>
#include <RtShmTransport.h>

/**
 * \brief `dr_controlboard_nwc_rtshm`: real-time shared-memory client side of the control board.
 *
 * Implements the same YARP control-board interfaces as `dr_controlboard_nwc_yarp`, but instead of
 * YARP ports it uses the RT-safe shared-memory transport described in docs/rtshm_design_doc.md:
 * a four-slot state snapshot ring for streamed measurements, per-family setpoint mailboxes for
 * high-rate commands, and a binary Unix-domain socket for the (non real-time) management RPC lane.
 */
class DinRailControlBoardNWCRtShm : public yarp::dev::IPidControl,
                                    public yarp::dev::IPositionControl,
                                    public yarp::dev::IVelocityControl,
                                    public yarp::dev::IEncodersTimed,
                                    public yarp::dev::IMotorEncoders,
                                    public yarp::dev::IMotor,
                                    public yarp::dev::IAmplifierControl,
                                    public yarp::dev::IControlLimits,
                                    public yarp::dev::IAxisInfo,
                                    public yarp::dev::IPreciselyTimed,
                                    public yarp::dev::IControlCalibration,
                                    public yarp::dev::ITorqueControl,
                                    public yarp::dev::IImpedanceControl,
                                    public yarp::dev::IControlMode,
                                    public yarp::dev::DeviceDriver,
                                    public yarp::dev::IPositionDirect,
                                    public yarp::dev::IInteractionMode,
                                    public yarp::dev::IRemoteCalibrator,
                                    public yarp::dev::IRemoteVariables,
                                    public yarp::dev::IPWMControl,
                                    public yarp::dev::ICurrentControl,
                                    public yarp::dev::IJointFault,
                                    public dinrail::IImpedanceAllSetPointsControl,
                                    public DinRailControlBoardNWCRtShm_ParamsParser
{
protected:
    dinrail::rtshm::SharedRegion m_region;
    dinrail::rtshm::UnixConnection m_conn;
    std::mutex m_rpcMutex;

    std::mutex m_stateMutex;
    dinrail::rtshm::StateSnapshot m_state;
    bool m_stateValid{false};

    std::uint32_t m_axes{0};
    std::uint64_t m_generation{0};
    bool m_open{false};
    double m_stateTimeout{0.5};

    mutable yarp::os::Stamp m_lastStamp;

    // Local shadows used to republish complete setpoint sets to the mailboxes and to answer the
    // impedance-all getters (design doc section 15).
    std::vector<double> m_shadowPosDirect;
    std::vector<double> m_shadowVelocity;
    std::vector<double> m_shadowTorque;
    std::vector<double> m_shadowCurrent;
    std::vector<double> m_shadowPwm;
    std::vector<double> m_shadowImpPos;
    std::vector<double> m_shadowImpVel;
    std::vector<double> m_shadowImpTorque;
    std::vector<double> m_shadowImpStiffness;
    std::vector<double> m_shadowImpDamping;

    // --- Transport helpers -------------------------------------------------
    bool handshake();
    bool refreshState();
    bool stateDoubleSingle(std::uint32_t field, int j, double* v);
    bool stateDoubleVector(std::uint32_t field, double* v);
    bool stateIntSingle(std::uint32_t field, int j, int* v);
    bool stateIntVector(std::uint32_t field, int* v);

    // Synchronous binary RPC over the management socket.
    bool rpc(std::uint32_t opcode,
             const std::function<void(dinrail::rtshm::PayloadWriter&)>& build,
             const std::function<bool(dinrail::rtshm::PayloadReader&)>& parse);
    bool rpcSimple(std::uint32_t opcode,
                   const std::function<void(dinrail::rtshm::PayloadWriter&)>& build);

    // Compact RPC shape helpers reused by many interface methods.
    bool rpcVoid(std::uint32_t op);
    bool rpcVoidJoint(std::uint32_t op, int j);
    bool rpcSetSingleD(std::uint32_t op, int j, double v);
    bool rpcGetSingleD(std::uint32_t op, int j, double* v);
    bool rpcSetSingleDD(std::uint32_t op, int j, double a, double b);
    bool rpcGetSingleDD(std::uint32_t op, int j, double* a, double* b);
    bool rpcSetArrayAll(std::uint32_t op, const double* v);
    bool rpcGetArrayAll(std::uint32_t op, double* v);
    bool rpcSetArrayGroup(std::uint32_t op, int n, const int* joints, const double* v);
    bool rpcGetArrayGroup(std::uint32_t op, int n, const int* joints, double* v);
    bool rpcGetPairAll(std::uint32_t op, double* a, double* b);

    void publishSetpointMailbox(std::uint32_t family);
    void publishImpedanceMailbox();
    std::uint64_t m_requestId{0};

public:
    DinRailControlBoardNWCRtShm() = default;
    DinRailControlBoardNWCRtShm(const DinRailControlBoardNWCRtShm&) = delete;
    DinRailControlBoardNWCRtShm(DinRailControlBoardNWCRtShm&&) = delete;
    DinRailControlBoardNWCRtShm& operator=(const DinRailControlBoardNWCRtShm&) = delete;
    DinRailControlBoardNWCRtShm& operator=(DinRailControlBoardNWCRtShm&&) = delete;
    ~DinRailControlBoardNWCRtShm() override = default;

    bool open(yarp::os::Searchable& config) override;
    bool close() override;

    bool getAxes(int* ax) override;

    // IPidControl
    bool setPid(const yarp::dev::PidControlTypeEnum& pidtype, int j, const yarp::dev::Pid& pid) override;
    bool setPids(const yarp::dev::PidControlTypeEnum& pidtype, const yarp::dev::Pid* pids) override;
    bool setPidReference(const yarp::dev::PidControlTypeEnum& pidtype, int j, double ref) override;
    bool setPidReferences(const yarp::dev::PidControlTypeEnum& pidtype, const double* refs) override;
    bool setPidErrorLimit(const yarp::dev::PidControlTypeEnum& pidtype, int j, double limit) override;
    bool setPidErrorLimits(const yarp::dev::PidControlTypeEnum& pidtype, const double* limits) override;
    bool getPidError(const yarp::dev::PidControlTypeEnum& pidtype, int j, double* err) override;
    bool getPidErrors(const yarp::dev::PidControlTypeEnum& pidtype, double* errs) override;
    bool getPid(const yarp::dev::PidControlTypeEnum& pidtype, int j, yarp::dev::Pid* pid) override;
    bool getPids(const yarp::dev::PidControlTypeEnum& pidtype, yarp::dev::Pid* pids) override;
    bool getPidReference(const yarp::dev::PidControlTypeEnum& pidtype, int j, double* ref) override;
    bool getPidReferences(const yarp::dev::PidControlTypeEnum& pidtype, double* refs) override;
    bool getPidErrorLimit(const yarp::dev::PidControlTypeEnum& pidtype, int j, double* limit) override;
    bool getPidErrorLimits(const yarp::dev::PidControlTypeEnum& pidtype, double* limits) override;
    bool resetPid(const yarp::dev::PidControlTypeEnum& pidtype, int j) override;
    bool disablePid(const yarp::dev::PidControlTypeEnum& pidtype, int j) override;
    bool enablePid(const yarp::dev::PidControlTypeEnum& pidtype, int j) override;
    bool isPidEnabled(const yarp::dev::PidControlTypeEnum& pidtype, int j, bool* enabled) override;
    bool getPidOutput(const yarp::dev::PidControlTypeEnum& pidtype, int j, double* out) override;
    bool getPidOutputs(const yarp::dev::PidControlTypeEnum& pidtype, double* outs) override;
    bool setPidOffset(const yarp::dev::PidControlTypeEnum& pidtype, int j, double v) override;

    // IEncoder / IEncodersTimed
    bool resetEncoder(int j) override;
    bool resetEncoders() override;
    bool setEncoder(int j, double val) override;
    bool setEncoders(const double* vals) override;
    bool getEncoder(int j, double* v) override;
    bool getEncoderTimed(int j, double* v, double* t) override;
    bool getEncoders(double* encs) override;
    bool getEncodersTimed(double* encs, double* ts) override;
    bool getEncoderSpeed(int j, double* sp) override;
    bool getEncoderSpeeds(double* spds) override;
    bool getEncoderAcceleration(int j, double* acc) override;
    bool getEncoderAccelerations(double* accs) override;

    // IRemoteVariables
    bool getRemoteVariable(std::string key, yarp::os::Bottle& val) override;
    bool setRemoteVariable(std::string key, const yarp::os::Bottle& val) override;
    bool getRemoteVariablesList(yarp::os::Bottle* listOfKeys) override;

    // IMotor
    bool getNumberOfMotors(int* num) override;
    bool getTemperature(int m, double* val) override;
    bool getTemperatures(double* vals) override;
    bool getTemperatureLimit(int m, double* val) override;
    bool setTemperatureLimit(int m, const double val) override;
    bool getGearboxRatio(int m, double* val) override;
    bool setGearboxRatio(int m, const double val) override;

    // IMotorEncoder
    bool resetMotorEncoder(int j) override;
    bool resetMotorEncoders() override;
    bool setMotorEncoder(int j, const double val) override;
    bool setMotorEncoderCountsPerRevolution(int m, const double cpr) override;
    bool getMotorEncoderCountsPerRevolution(int m, double* cpr) override;
    bool setMotorEncoders(const double* vals) override;
    bool getMotorEncoder(int j, double* v) override;
    bool getMotorEncoderTimed(int j, double* v, double* t) override;
    bool getMotorEncoders(double* encs) override;
    bool getMotorEncodersTimed(double* encs, double* ts) override;
    bool getMotorEncoderSpeed(int j, double* sp) override;
    bool getMotorEncoderSpeeds(double* spds) override;
    bool getMotorEncoderAcceleration(int j, double* acc) override;
    bool getMotorEncoderAccelerations(double* accs) override;
    bool getNumberOfMotorEncoders(int* num) override;

    // IPreciselyTimed
    yarp::os::Stamp getLastInputStamp() override;

    // IPositionControl
    bool positionMove(int j, double ref) override;
    bool positionMove(const int n_joint, const int* joints, const double* refs) override;
    bool positionMove(const double* refs) override;
    bool getTargetPosition(const int joint, double* ref) override;
    bool getTargetPositions(double* refs) override;
    bool getTargetPositions(const int n_joint, const int* joints, double* refs) override;
    bool relativeMove(int j, double delta) override;
    bool relativeMove(const int n_joint, const int* joints, const double* refs) override;
    bool relativeMove(const double* deltas) override;
    bool checkMotionDone(int j, bool* flag) override;
    bool checkMotionDone(const int n_joint, const int* joints, bool* flag) override;
    bool checkMotionDone(bool* flag) override;
    bool setRefSpeed(int j, double sp) override;
    bool setRefSpeeds(const int n_joint, const int* joints, const double* spds) override;
    bool setRefSpeeds(const double* spds) override;
    bool setRefAcceleration(int j, double acc) override;
    bool setRefAccelerations(const int n_joint, const int* joints, const double* accs) override;
    bool setRefAccelerations(const double* accs) override;
    bool getRefSpeed(int j, double* ref) override;
    bool getRefSpeeds(const int n_joint, const int* joints, double* spds) override;
    bool getRefSpeeds(double* spds) override;
    bool getRefAcceleration(int j, double* acc) override;
    bool getRefAccelerations(const int n_joint, const int* joints, double* accs) override;
    bool getRefAccelerations(double* accs) override;
    bool stop(int j) override;
    bool stop(const int len, const int* val1) override;
    bool stop() override;

    // IJointFault
    bool getLastJointFault(int j, int& fault, std::string& message) override;

    // IVelocityControl
    bool velocityMove(int j, double v) override;
    bool velocityMove(const double* v) override;

    // IAmplifierControl
    bool enableAmp(int j) override;
    bool disableAmp(int j) override;
    bool getAmpStatus(int* st) override;
    bool getAmpStatus(int j, int* st) override;
    bool setMaxCurrent(int j, double v) override;
    bool getMaxCurrent(int j, double* v) override;
    bool getNominalCurrent(int m, double* val) override;
    bool setNominalCurrent(int m, const double val) override;
    bool getPeakCurrent(int m, double* val) override;
    bool setPeakCurrent(int m, const double val) override;
    bool getPWM(int m, double* val) override;
    bool getPWMLimit(int m, double* val) override;
    bool setPWMLimit(int m, const double val) override;
    bool getPowerSupplyVoltage(int m, double* val) override;

    // IControlLimits
    bool setLimits(int axis, double min, double max) override;
    bool getLimits(int axis, double* min, double* max) override;
    bool setVelLimits(int axis, double min, double max) override;
    bool getVelLimits(int axis, double* min, double* max) override;

    // IAxisInfo
    bool getAxisName(int j, std::string& name) override;
    bool getJointType(int j, yarp::dev::JointTypeEnum& type) override;

    // IControlCalibration
    bool calibrateRobot() override;
    bool abortCalibration() override;
    bool abortPark() override;
    bool park(bool wait = true) override;
    bool calibrateAxisWithParams(int j, unsigned int ui, double v1, double v2, double v3) override;
    bool setCalibrationParameters(int j, const yarp::dev::CalibrationParameters& params) override;
    bool calibrationDone(int j) override;

    // ITorqueControl
    bool getRefTorque(int j, double* t) override;
    bool getRefTorques(double* t) override;
    bool setRefTorques(const double* t) override;
    bool setRefTorque(int j, double v) override;
    bool setRefTorques(const int n_joint, const int* joints, const double* t) override;
    bool setMotorTorqueParams(int j, const yarp::dev::MotorTorqueParameters params) override;
    bool getMotorTorqueParams(int j, yarp::dev::MotorTorqueParameters* params) override;
    bool getTorque(int j, double* t) override;
    bool getTorques(double* t) override;
    bool getTorqueRange(int j, double* min, double* max) override;
    bool getTorqueRanges(double* min, double* max) override;

    // IImpedanceControl
    bool getImpedance(int j, double* stiffness, double* damping) override;
    bool getImpedanceOffset(int j, double* offset) override;
    bool setImpedance(int j, double stiffness, double damping) override;
    bool setImpedanceOffset(int j, double offset) override;
    bool getCurrentImpedanceLimit(
        int j, double* min_stiff, double* max_stiff, double* min_damp, double* max_damp) override;

    // dinrail::IImpedanceAllSetPointsControl
    bool setSetPoint(
        int j, double pos, double vel, double torque, double stiffness, double damping) override;
    bool setSetPoints(const dinrail::VectorProxy<const int>::Ref jointIndeces,
                      const dinrail::VectorProxy<const double>::Ref pos,
                      const dinrail::VectorProxy<const double>::Ref vel,
                      const dinrail::VectorProxy<const double>::Ref torque,
                      const dinrail::VectorProxy<const double>::Ref stiffness,
                      const dinrail::VectorProxy<const double>::Ref damping) override;
    bool setSetPoints(const dinrail::VectorProxy<const double>::Ref pos,
                      const dinrail::VectorProxy<const double>::Ref vel,
                      const dinrail::VectorProxy<const double>::Ref torque,
                      const dinrail::VectorProxy<const double>::Ref stiffness,
                      const dinrail::VectorProxy<const double>::Ref damping) override;
    bool
    getSetPoint(int j, double& pos, double& vel, double& torque, double& stiffness, double& damping)
        override;
    bool getSetPoints(const dinrail::VectorProxy<const int>::Ref jointIndeces,
                      dinrail::VectorProxy<double>::Ref pos,
                      dinrail::VectorProxy<double>::Ref vel,
                      dinrail::VectorProxy<double>::Ref torque,
                      dinrail::VectorProxy<double>::Ref stiffness,
                      dinrail::VectorProxy<double>::Ref damping) override;
    bool getSetPoints(dinrail::VectorProxy<double>::Ref pos,
                      dinrail::VectorProxy<double>::Ref vel,
                      dinrail::VectorProxy<double>::Ref torque,
                      dinrail::VectorProxy<double>::Ref stiffness,
                      dinrail::VectorProxy<double>::Ref damping) override;

    // IControlMode
    bool getControlMode(int j, int* mode) override;
    bool getControlModes(int* modes) override;
    bool getControlModes(const int n_joint, const int* joints, int* modes) override;
    bool setControlMode(const int j, const int mode) override;
    bool setControlModes(const int n_joint, const int* joints, int* modes) override;
    bool setControlModes(int* modes) override;

    // IPositionDirect
    bool setPosition(int j, double ref) override;
    bool setPositions(const int n_joint, const int* joints, const double* refs) override;
    bool setPositions(const double* refs) override;
    bool getRefPosition(const int joint, double* ref) override;
    bool getRefPositions(double* refs) override;
    bool getRefPositions(const int n_joint, const int* joints, double* refs) override;

    // IVelocityControl
    bool velocityMove(const int n_joint, const int* joints, const double* spds) override;
    bool getRefVelocity(const int joint, double* vel) override;
    bool getRefVelocities(double* vels) override;
    bool getRefVelocities(const int n_joint, const int* joints, double* vels) override;

    // IInteractionMode
    bool getInteractionMode(int axis, yarp::dev::InteractionModeEnum* mode) override;
    bool
    getInteractionModes(int n_joints, int* joints, yarp::dev::InteractionModeEnum* modes) override;
    bool getInteractionModes(yarp::dev::InteractionModeEnum* modes) override;
    bool setInteractionMode(int axis, yarp::dev::InteractionModeEnum mode) override;
    bool
    setInteractionModes(int n_joints, int* joints, yarp::dev::InteractionModeEnum* modes) override;
    bool setInteractionModes(yarp::dev::InteractionModeEnum* modes) override;

    // IRemoteCalibrator
    bool isCalibratorDevicePresent(bool* isCalib) override;
    bool calibrateSingleJoint(int j) override;
    bool calibrateWholePart() override;
    bool homingSingleJoint(int j) override;
    bool homingWholePart() override;
    bool parkSingleJoint(int j, bool _wait = true) override;
    bool parkWholePart() override;
    bool quitCalibrate() override;
    bool quitPark() override;

    // ICurrentControl
    bool getRefCurrents(double* t) override;
    bool getRefCurrent(int j, double* t) override;
    bool setRefCurrents(const double* refs) override;
    bool setRefCurrent(int j, double ref) override;
    bool setRefCurrents(const int n_joint, const int* joints, const double* refs) override;
    bool getCurrents(double* vals) override;
    bool getCurrent(int j, double* val) override;
    bool getCurrentRange(int j, double* min, double* max) override;
    bool getCurrentRanges(double* min, double* max) override;

    // IPWMControl
    bool setRefDutyCycle(int j, double v) override;
    bool setRefDutyCycles(const double* v) override;
    bool getRefDutyCycle(int j, double* ref) override;
    bool getRefDutyCycles(double* refs) override;
    bool getDutyCycle(int j, double* out) override;
    bool getDutyCycles(double* outs) override;
};

#endif // DINRAIL_CONTROLBOARD_NWC_RTSHM_H
