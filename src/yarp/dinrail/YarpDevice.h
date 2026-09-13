// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPDEVICE_H
#define DINRAIL_YARPDEVICE_H

#include <dinrail/Device.h>
#include <dinrail/YarpPropertyConverter.h>
#include <yarp/dev/DeviceDriver.h>
#include <yarp/os/Property.h>

#include <cstddef>
#include <type_traits>

namespace dinrail
{
namespace detail
{
template <class Adapter> bool bindAdapter(Adapter& adapter, dinrail::Device& device)
{
    typename Adapter::SourceInterface* source = nullptr;
    if (!device.view(source))
    {
        return false;
    }
    adapter.bind(*source);
    return true;
}

template <class Adapter> void unbindAdapter(Adapter& adapter)
{
    adapter.unbind();
}

template <class Adapter, class... All>
constexpr std::size_t targetProviderCount
    = (std::size_t{0} + ...
       + std::size_t{std::is_base_of_v<typename Adapter::TargetInterface, All>});

} // namespace detail

/** Own a native device and expose explicitly selected adapters through YARP.
 * The backing native device is opened through dinrail::Device using the YARP
 * configuration.
 */
template <class... Adapters>
class YarpDevice : public yarp::dev::DeviceDriver, public Adapters...
{
    static_assert((!std::is_abstract_v<Adapters> && ...),
                  "YarpDevice: selected adapter is abstract");
    static_assert((std::is_default_constructible_v<Adapters> && ...),
                  "YarpDevice: selected adapter must be default constructible");
    static_assert(((detail::targetProviderCount<Adapters, Adapters...> == 1) && ...),
                  "YarpDevice: selected adapters expose an ambiguous interface; select only one "
                  "provider");
    static_assert(sizeof...(Adapters) > 0,
                  "YarpDevice: explicitly list adapters to expose");

public:
    dinrail::Device& device()
    {
        return m_device;
    }
    const dinrail::Device& device() const
    {
        return m_device;
    }
    bool open(yarp::os::Searchable& config) override
    {
        close();
        yarp::os::Property property;
        property.fromString(config.toString());
        if (!m_device.openNative(dinrail::YarpPropertyConverter::toDinrailParameters(property)))
        {
            return false;
        }
        if ((detail::bindAdapter(static_cast<Adapters&>(*this), m_device) && ...))
        {
            return true;
        }
        (detail::unbindAdapter(static_cast<Adapters&>(*this)), ...);
        m_device.close();
        return false;
    }
    bool close() override
    {
        (detail::unbindAdapter(static_cast<Adapters&>(*this)), ...);
        return !m_device.isValid() || m_device.close();
    }

private:
    dinrail::Device m_device;
};

} // namespace dinrail
#endif
