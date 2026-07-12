// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_CONTEXT_H
#define DINRAIL_CONTEXT_H

#include <memory>
#include <typeinfo>

namespace dinrail
{

class Device;
class IDevice;
class Parameters;

/**
 * Shared runtime state used to load device and compatibility plug-ins.
 *
 * A RuntimeContext owns the plug-in cache and interface-adapter registry. Copies are
 * lightweight handles to the same state, so passing a RuntimeContext to a Device is
 * safe even when the original RuntimeContext object goes out of scope.
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
    static const RuntimeContext& get_default();

private:
    struct Impl;
    std::shared_ptr<Impl> m_pimpl;

    std::unique_ptr<IDevice> createDevice(const Parameters& config);
    void* queryAdapter(IDevice* device, const std::type_info& interfaceType) const;

    friend class Device;
};

} // namespace dinrail

#endif // DINRAIL_CONTEXT_H
