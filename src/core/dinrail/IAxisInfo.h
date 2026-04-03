// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IAXISINFO_H
#define DINRAIL_IAXISINFO_H

#include <string>

namespace dinrail
{

/**
 * @brief Joint classification used by axis information interfaces.
 */
enum class JointType
{
    /** Revolute joint (rotational degree of freedom). */
    REVOLUTE = 1,
    /** Prismatic joint (translational degree of freedom). */
    PRISMATIC = 2,
    /** Joint type is not available or cannot be determined. */
    UNKNOWN = 0
};

/**
 * @brief Interface exposing metadata for device axes.
 */
class IAxisInfo
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IAxisInfo() = default;

    /**
     * @brief Get the number of controlled axes.
     * @param ax Output pointer receiving the number of axes.
     * @return true on success, false otherwise.
     */
    virtual bool getAxes(int* ax) = 0;

    /**
     * @brief Get the name associated with a given axis.
     * @param axis Zero-based axis index.
     * @param name Output string receiving the axis name.
     * @return true on success, false otherwise.
     */
    virtual bool getAxisName(int axis, std::string& name) = 0;

    /**
     * @brief Get the joint type of a given axis.
     * @param axis Zero-based axis index.
     * @param type Output value receiving the joint type.
     * @return true on success, false otherwise.
     */
    virtual bool getJointType(int axis, JointType& type) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IAXISINFO_H
