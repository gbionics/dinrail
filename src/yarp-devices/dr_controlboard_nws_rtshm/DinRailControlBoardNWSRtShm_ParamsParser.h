/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DINRAIL_CONTROLBOARD_NWS_RTSHM_PARAMSPARSER_H
#define DINRAIL_CONTROLBOARD_NWS_RTSHM_PARAMSPARSER_H

#include <string>
#include <vector>

#include <yarp/dev/IDeviceDriverParams.h>
#include <yarp/os/Searchable.h>
#include <yarp/os/Value.h>

/**
 * Parameters parser for DinRailControlBoardNWSRtShm.
 *
 * | Parameter name                          | Type   | Default | Required | Description               |
 * |:---------------------------------------:|:------:|:-------:|:--------:|:-------------------------:|
 * | name                                    | string | -       | 1        | Endpoint name (socket).   |
 * | endpoint                                | string | -       | 0        | Explicit socket override. |
 * | period                                  | double | 0.01    | 0        | Service thread period (s).|
 * | lock_shared_memory                      | bool   | false   | 0        | mlock the shared region.  |
 * | emulate_impedance_all_setpoints_control | bool   | false   | 0        | Emulate impedance-all.    |
 */
class DinRailControlBoardNWSRtShm_ParamsParser : public yarp::dev::IDeviceDriverParams
{
public:
    DinRailControlBoardNWSRtShm_ParamsParser() = default;
    ~DinRailControlBoardNWSRtShm_ParamsParser() override = default;

    const std::string m_device_classname{"DinRailControlBoardNWSRtShm"};
    const std::string m_device_name{"dr_controlboard_nws_rtshm"};
    std::string m_provided_configuration;

    std::string m_name{};
    std::string m_endpoint{};
    double m_period{0.01};
    bool m_lock_shared_memory{false};
    bool m_emulate_impedance_all_setpoints_control{false};

    bool parseParams(const yarp::os::Searchable& config) override
    {
        m_provided_configuration = config.toString();
        if (config.check("name"))
        {
            m_name = config.find("name").asString();
        }
        if (config.check("endpoint"))
        {
            m_endpoint = config.find("endpoint").asString();
        }
        if (config.check("period"))
        {
            m_period = config.find("period").asFloat64();
        }
        if (config.check("lock_shared_memory"))
        {
            m_lock_shared_memory = config.find("lock_shared_memory").asBool();
        }
        if (config.check("emulate_impedance_all_setpoints_control"))
        {
            m_emulate_impedance_all_setpoints_control
                = config.find("emulate_impedance_all_setpoints_control").asBool();
        }
        return !m_name.empty();
    }

    std::string getDeviceClassName() const override
    {
        return m_device_classname;
    }
    std::string getDeviceName() const override
    {
        return m_device_name;
    }
    std::string getDocumentationOfDeviceParams() const override
    {
        return "dr_controlboard_nws_rtshm: real-time shared-memory control-board network server.";
    }
    std::vector<std::string> getListOfParams() const override
    {
        return {"name",
                "endpoint",
                "period",
                "lock_shared_memory",
                "emulate_impedance_all_setpoints_control"};
    }
    bool getParamValue(const std::string& paramName, std::string& paramValue) const override
    {
        if (paramName == "name")
        {
            paramValue = m_name;
        } else if (paramName == "endpoint")
        {
            paramValue = m_endpoint;
        } else if (paramName == "period")
        {
            paramValue = std::to_string(m_period);
        } else if (paramName == "lock_shared_memory")
        {
            paramValue = m_lock_shared_memory ? "true" : "false";
        } else if (paramName == "emulate_impedance_all_setpoints_control")
        {
            paramValue = m_emulate_impedance_all_setpoints_control ? "true" : "false";
        } else
        {
            return false;
        }
        return true;
    }
    std::string getConfiguration() const override
    {
        return m_provided_configuration;
    }
};

#endif // DINRAIL_CONTROLBOARD_NWS_RTSHM_PARAMSPARSER_H
