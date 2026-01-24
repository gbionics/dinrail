// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPPROPERTYCONVERTER_H
#define DINRAIL_YARPPROPERTYCONVERTER_H

#include <dinrail/Property.h>

namespace yarp::os { class Property; }

namespace dinrail
{

/**
 * @brief Utility class for converting dinrail::Property to yarp::os::Property.
 */
class YarpPropertyConverter
{
public:
    /**
     * @brief Convert a dinrail::Property to a yarp::os::Property.
     * 
     * This method performs a deep conversion, recursively handling all
     * nested groups with arbitrary depth.
     * 
     * @param dinrailProp The dinrail property to convert
     * @return The converted YARP property
     */
    static yarp::os::Property toYarpProperty(const Property& dinrailProp);

private:
    YarpPropertyConverter() = delete;
    
    /**
     * @brief Recursively convert a dinrail::Property to a yarp::os::Property.
     * @param dinrailProp The source property
     * @param yarpProp The destination property (modified in-place)
     */
    static void convertRecursive(const Property& dinrailProp, yarp::os::Property& yarpProp);
};

} // namespace dinrail

#endif // DINRAIL_YARPPROPERTYCONVERTER_H
