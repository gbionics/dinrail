// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_INTERFACEADAPTER_H
#define DINRAIL_INTERFACEADAPTER_H

#include <dinrail/IDevice.h>
#include <dinrail/IInterfaceAdapter.h>
#include <dinrail/IInterfaceView.h>

#include <cassert>
#include <memory>

namespace dinrail
{

template <class Interface> Interface* viewSourceInterface(IDevice& device)
{
    if (auto* interface = dynamic_cast<Interface*>(&device))
    {
        return interface;
    }

    auto* interfaceView = dynamic_cast<IInterfaceView*>(&device);
    return interfaceView == nullptr
               ? nullptr
               : static_cast<Interface*>(interfaceView->viewInterface(typeid(Interface)));
}

template <class Target, class Source>
class InterfaceAdapterBase : public IInterfaceAdapter, public Target
{
public:
    using SourceInterface = Source;
    using TargetInterface = Target;

    InterfaceAdapterBase() = default;
    explicit InterfaceAdapterBase(Source& source)
    {
        bind(source);
    }

    void bind(Source& source)
    {
        m_source = &source;
    }

    void unbind()
    {
        m_source = nullptr;
    }

    void* getInterface() final
    {
        return static_cast<Target*>(this);
    }

protected:
    Source& source()
    {
        assert(m_source != nullptr);
        return *m_source;
    }

    const Source& source() const
    {
        assert(m_source != nullptr);
        return *m_source;
    }

private:
    Source* m_source{nullptr};
};

template <class...> inline constexpr bool dependent_false_v = false;

template <class Target, class Source> class InterfaceAdapter
{
    static_assert(dependent_false_v<Target, Source>,
                  "InterfaceAdapter must be explicitly specialized");
};

} // namespace dinrail

#endif // DINRAIL_INTERFACEADAPTER_H
