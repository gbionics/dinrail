// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DEVICE_H
#define DINRAIL_DEVICE_H

#include <dinrail/AdapterRegistry.h>
#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>

#include <memory>
#include <string>
#include <typeinfo>

namespace dinrail
{

class Device
{
public:
    Device();
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&&) = default;
    Device& operator=(Device&&) = default;

    bool open(const Parameters& config);
    bool close();
    bool isValid() const;

    template <class T> bool view(T*& x)
    {
        x = nullptr;

        IDevice* impl = getImplementation();
        if (!impl)
        {
            return false;
        }

        void* ptr = AdapterRegistry::queryAdapter(impl, typeid(T));
        if (ptr != nullptr)
        {
            x = static_cast<T*>(ptr);
            return true;
        }

        T* v = dynamic_cast<T*>(impl);
        if (v != nullptr)
        {
            x = v;
            return true;
        }

        return false;
    }

    IDevice* getImplementation();
    std::string getDeviceName() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;
};

} // namespace dinrail

#endif // DINRAIL_DEVICE_H
