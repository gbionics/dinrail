// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPDEVICEWRAPPER_H
#define DINRAIL_YARPDEVICEWRAPPER_H

#include <dinrail/IDevice.h>
#include <dinrail/IPositionDirect.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IControlMode.h>

#include <yarp/dev/PolyDriver.h>

#include <memory>

namespace dinrail
{

// Forward declarations of adapters
class YarpPositionDirectAdapter;
class YarpEncodersAdapter;
class YarpControlModeAdapter;

/**
 * @brief Wrapper that adapts a YARP PolyDriver to the dinrail::IDevice interface.
 *
 * This wrapper allows YARP devices to be used transparently through dinrail.
 * It uses composition with separate adapter classes for each interface, making
 * it easy to add support for new interfaces without modifying this class.
 * 
 * Interface access is handled through the view<T>() method, which returns the
 * appropriate adapter without requiring YarpDeviceWrapper to inherit from each interface.
 */
class YarpDeviceWrapper : public IDevice
{
public:
    explicit YarpDeviceWrapper(std::unique_ptr<yarp::dev::PolyDriver> yarpDevice);
    ~YarpDeviceWrapper() override;

    // IDevice interface
    bool open(const Property& config) override;
    bool close() override;
    IDevice* allocateInstance() const override;

    /**
     * @brief Get adapter for a specific interface type.
     * 
     * This is called by adapter factory functions to retrieve
     * interface adapters for this YARP device wrapper.
     * 
     * @param interfaceType The type_info of the requested interface.
     * @return Pointer to the adapter, or nullptr if not available.
     */
    void* getAdapter(const std::type_info& interfaceType);

private:
    std::unique_ptr<yarp::dev::PolyDriver> m_yarpDevice;
    
    // Interface adapters - each handles conversion for one interface
    std::unique_ptr<YarpPositionDirectAdapter> m_positionDirectAdapter;
    std::unique_ptr<YarpEncodersAdapter> m_encodersAdapter;
    std::unique_ptr<YarpControlModeAdapter> m_controlModeAdapter;
};

} // namespace dinrail

#endif // DINRAIL_YARPDEVICEWRAPPER_H
