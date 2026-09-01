// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IJOINTFAULT_H
#define DINRAIL_IJOINTFAULT_H

#include <string>

namespace dinrail
{

/**
 * @brief Interface for retrieving the last fault reported by a joint.
 *
 * This interface mirrors `yarp::dev::IJointFault`.
 */
class IJointFault
{
public:
    /** @brief Virtual destructor. */
    virtual ~IJointFault();

    /**
     * @brief Get the last fault reported by one joint.
     * @param j Joint index.
     * @param fault Output variable receiving the fault code.
     * @param message Output string receiving the fault description.
     * @return True on success, false otherwise.
     */
    virtual bool getLastJointFault(int j, int& fault, std::string& message) = 0;
};

/** @brief Simulation interface used to provide joint fault information. */
class IJointFaultSimulation
{
public:
    /** @brief Virtual destructor. */
    virtual ~IJointFaultSimulation();

    /**
     * @brief Set the simulated last fault reported by one joint.
     * @param j Joint index.
     * @param fault Fault code.
     * @param message Fault description.
     * @return True on success, false otherwise.
     */
    virtual bool setLastJointFault(int j, int fault, const std::string& message) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IJOINTFAULT_H
