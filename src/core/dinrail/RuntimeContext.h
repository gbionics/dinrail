// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_CONTEXT_H
#define DINRAIL_CONTEXT_H

#include <functional>
#include <memory>

namespace dinrail
{

class Device;
class IDevice;
class Parameters;

// Deleter for unique_ptr that calls a stored destroy function instead of delete.
// This is used to manage the lifetime of devices created via plugin factories.
template <class T> struct FactoryDeleter
{
    std::function<void(T*)> destroy_fn;

    void operator()(T* p) const noexcept
    {
        if (p != nullptr && destroy_fn)
        {
            destroy_fn(p);
        }
    }
};

template <class T> using FactoryUniquePtr = std::unique_ptr<T, FactoryDeleter<T>>;

/**
 * Shared runtime state used to load device plug-ins.
 *
 * A RuntimeContext owns the plug-in cache. Copies are lightweight handles to
 * the same state, so passing a RuntimeContext to a Device is safe even when
 * the original RuntimeContext object goes out of scope.
 */
class RuntimeContext
{
public:
    RuntimeContext();
    ~RuntimeContext();

    RuntimeContext(const RuntimeContext&) = default;
    RuntimeContext& operator=(const RuntimeContext&) = default;
    RuntimeContext(RuntimeContext&&) noexcept = default;
    RuntimeContext& operator=(RuntimeContext&&) noexcept = default;

    /**
     * Process-wide context used by the simple constructors.
     */
    static const RuntimeContext& getDefault();

private:
    struct Impl;
    std::shared_ptr<Impl> m_pimpl;

    /**
     * @brief Load and open a device from the provided configuration.
     *
     * Looks up (or loads) the plug-in library for the device named by
     * `config["device"]`, creates an instance, and calls `IDevice::open()`.
     * Returns nullptr on any failure.
     */
    FactoryUniquePtr<IDevice> createDevice(const Parameters& config);

    friend class Device;
};

} // namespace dinrail

#endif // DINRAIL_CONTEXT_H
