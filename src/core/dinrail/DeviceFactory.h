// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DEVICEFACTORY_H
#define DINRAIL_DEVICEFACTORY_H

#include <dinrail/IDevice.h>

namespace dinrail
{

/**
 * @brief Template helper base class to avoid boilerplate allocateInstance implementations.
 *
 * Plugin developers can inherit from `DeviceFactory<ConcreteDevice>` instead of manually
 * implementing `allocateInstance()` in their device class.
 *
 * Example usage (in MyDevice.h):
 *
 * ```cpp
 * class MyDevice : public dinrail::DeviceFactory<MyDevice>, public dinrail::IAxisInfo
 * {
 * public:
 *     bool open(const Parameters& config) override;
 *     bool close() override;
 *     // allocateInstance() is provided by DeviceFactory<MyDevice>
 * };
 * ```
 *
 * The factory will automatically provide:
 * ```cpp
 * IDevice* allocateInstance() const override { return new ConcreteDevice(); }
 * ```
 *
 * @tparam ConcreteDevice The concrete device class implementing this factory.
 */
template <typename ConcreteDevice>
class DeviceFactory : public IDevice
{
public:
    /**
     * @brief Allocate a new instance of the concrete device.
     * @return A new instance of ConcreteDevice allocated on the heap.
     */
    IDevice* allocateInstance() const override
    {
        return new ConcreteDevice();
    }
};

} // namespace dinrail

#endif // DINRAIL_DEVICEFACTORY_H
