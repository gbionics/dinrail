/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DINRAIL_CONTROLBOARD_NWC_RTSHM_PARAMSPARSER_H
#define DINRAIL_CONTROLBOARD_NWC_RTSHM_PARAMSPARSER_H

#include <string>
#include <vector>

#include <yarp/dev/IDeviceDriverParams.h>
#include <yarp/os/Searchable.h>
#include <yarp/os/Value.h>

/**
 * Parameters parser for DinRailControlBoardNWCRtShm.
 *
 * | Parameter name    | Type   | Default | Required | Description                                  |
 * |:-----------------:|:------:|:-------:|:--------:|:--------------------------------------------:|
 * | remote            | string | -       | 1        | Name shared with the NWS (socket endpoint).  |
 * | local             | string | -       | 0        | Local name (informational only).             |
 * | endpoint          | string | -       | 0        | Explicit socket path override.               |
 * | connect_timeout_ms| int    | 5000    | 0        | Timeout to connect to the NWS.               |
 * | rpc_timeout_ms    | int    | 5000    | 0        | Timeout for a management RPC.                |
 * | state_timeout_ms  | int    | 500     | 0        | Max age of a state snapshot to be valid.     |
 */
class DinRailControlBoardNWCRtShm_ParamsParser : public yarp::dev::IDeviceDriverParams
{
public:
    DinRailControlBoardNWCRtShm_ParamsParser() = default;
    ~DinRailControlBoardNWCRtShm_ParamsParser() override = default;

    const std::string m_device_classname{"DinRailControlBoardNWCRtShm"};
    const std::string m_device_name{"dr_controlboard_nwc_rtshm"};
    std::string m_provided_configuration;

    std::string m_remote{};
    std::string m_local{};
    std::string m_endpoint{};
    int m_connect_timeout_ms{5000};
    int m_rpc_timeout_ms{5000};
    int m_state_timeout_ms{500};

    bool parseParams(const yarp::os::Searchable& config) override
    {
        m_provided_configuration = config.toString();
        if (config.check("remote"))
        {
            m_remote = config.find("remote").asString();
        }
        if (config.check("local"))
        {
            m_local = config.find("local").asString();
        }
        if (config.check("endpoint"))
        {
            m_endpoint = config.find("endpoint").asString();
        }
        if (config.check("connect_timeout_ms"))
        {
            m_connect_timeout_ms = config.find("connect_timeout_ms").asInt32();
        }
        if (config.check("rpc_timeout_ms"))
        {
            m_rpc_timeout_ms = config.find("rpc_timeout_ms").asInt32();
        }
        if (config.check("state_timeout_ms"))
        {
            m_state_timeout_ms = config.find("state_timeout_ms").asInt32();
        }
        return !m_remote.empty();
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
        return "dr_controlboard_nwc_rtshm: real-time shared-memory control-board network client.";
    }
    std::vector<std::string> getListOfParams() const override
    {
        return {"remote",
                "local",
                "endpoint",
                "connect_timeout_ms",
                "rpc_timeout_ms",
                "state_timeout_ms"};
    }
    bool getParamValue(const std::string& paramName, std::string& paramValue) const override
    {
        if (paramName == "remote")
        {
            paramValue = m_remote;
        } else if (paramName == "local")
        {
            paramValue = m_local;
        } else if (paramName == "endpoint")
        {
            paramValue = m_endpoint;
        } else if (paramName == "connect_timeout_ms")
        {
            paramValue = std::to_string(m_connect_timeout_ms);
        } else if (paramName == "rpc_timeout_ms")
        {
            paramValue = std::to_string(m_rpc_timeout_ms);
        } else if (paramName == "state_timeout_ms")
        {
            paramValue = std::to_string(m_state_timeout_ms);
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

#endif // DINRAIL_CONTROLBOARD_NWC_RTSHM_PARAMSPARSER_H
