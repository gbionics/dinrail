// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPPROPERTYCONVERTER_H
#define DINRAIL_YARPPROPERTYCONVERTER_H

#include <dinrail/Parameters.h>

namespace yarp::os { class Property; }

namespace dinrail
{

/**
 * @brief Utility class for converting dinrail::Parameters to yarp::os::Property.
 */
class YarpPropertyConverter
{
public:
    /**
     * @brief Convert a dinrail::Parameters to a yarp::os::Property.
     *
     * This method performs a deep conversion, recursively handling all
     * nested groups with arbitrary depth.
     *
     * @param dinrailParams The dinrail parameters to convert
     * @return The converted YARP property
     */
    static yarp::os::Property toYarpProperty(const Parameters& dinrailParams);

    /**
     * @brief Convert a yarp::os::Property to dinrail::Parameters.
     *
     * This method performs a deep conversion, recursively handling nested
     * groups and scalar/vector values encoded in the property.
     *
     * @param yarpProp The YARP property to convert
     * @return The converted dinrail parameters
     */
    static Parameters toDinrailParameters(const yarp::os::Property& yarpProp);

private:
    YarpPropertyConverter() = delete;
    
    /**
     * @brief Recursively convert a dinrail::Parameters to a yarp::os::Property.
     * @param dinrailParams The source parameters
     * @param yarpProp The destination property (modified in-place)
     */
    static void convertRecursive(const Parameters& dinrailParams, yarp::os::Property& yarpProp);
};

} // namespace dinrail

#endif // DINRAIL_YARPPROPERTYCONVERTER_H
