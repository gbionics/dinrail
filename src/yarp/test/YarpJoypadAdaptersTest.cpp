// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>
#include <dinrail/YarpInteropPlugin.h>
#include <dinrail/YarpJoypadAdapters.h>

#include <array>

namespace
{
struct JoypadState
{
    bool available{true};
    std::array<double, 2> axes{0.25, -0.75};
    std::array<float, 3> buttons{0.0f, 1.0f, 0.0f};
    std::array<unsigned char, 1> hats{4};
};

class NativeJoypad final : public dinrail::IDevice, public dinrail::IJoypadControl
{
public:
    JoypadState state;
    bool open(const dinrail::Parameters&) override
    {
        return true;
    }
    bool close() override
    {
        return true;
    }
    bool getAxisCount(unsigned int& count) override
    {
        count = state.axes.size();
        return state.available;
    }
    bool getButtonCount(unsigned int& count) override
    {
        count = state.buttons.size();
        return state.available;
    }
    bool getHatCount(unsigned int& count) override
    {
        count = state.hats.size();
        return state.available;
    }
    bool getAxis(unsigned int index, double& value) override
    {
        if (!state.available || index >= state.axes.size())
        {
            return false;
        }
        value = state.axes[index];
        return true;
    }
    bool getButton(unsigned int index, float& value) override
    {
        if (!state.available || index >= state.buttons.size())
        {
            return false;
        }
        value = state.buttons[index];
        return true;
    }
    bool getHat(unsigned int index, unsigned char& value) override
    {
        if (!state.available || index >= state.hats.size())
        {
            return false;
        }
        value = state.hats[index];
        return true;
    }
    bool reconnect() override
    {
        state.available = true;
        return true;
    }
    bool getLastEvent(dinrail::JoypadDeviceEvent& event) override
    {
        event = dinrail::JoypadDeviceEvent::Connected;
        return true;
    }
};

class YarpJoypad final : public yarp::dev::IJoypadController
{
public:
    JoypadState state;
    bool getAxisCount(unsigned int& count) override
    {
        count = state.axes.size();
        return state.available;
    }
    bool getButtonCount(unsigned int& count) override
    {
        count = state.buttons.size();
        return state.available;
    }
    bool getHatCount(unsigned int& count) override
    {
        count = state.hats.size();
        return state.available;
    }
    bool getAxis(unsigned int index, double& value) override
    {
        if (!state.available || index >= state.axes.size())
        {
            return false;
        }
        value = state.axes[index];
        return true;
    }
    bool getButton(unsigned int index, float& value) override
    {
        if (!state.available || index >= state.buttons.size())
        {
            return false;
        }
        value = state.buttons[index];
        return true;
    }
    bool getHat(unsigned int index, unsigned char& value) override
    {
        if (!state.available || index >= state.hats.size())
        {
            return false;
        }
        value = state.hats[index];
        return true;
    }
    bool getTrackballCount(unsigned int& count) override
    {
        count = 1;
        return state.available;
    }
    bool getTouchSurfaceCount(unsigned int& count) override
    {
        count = 1;
        return state.available;
    }
    bool getStickCount(unsigned int& count) override
    {
        count = 1;
        return state.available;
    }
    bool getStickDoF(unsigned int, unsigned int& count) override
    {
        count = 2;
        return state.available;
    }
    bool getTrackball(unsigned int, yarp::sig::Vector&) override
    {
        return state.available;
    }
    bool getStick(unsigned int, yarp::sig::Vector&, JoypadCtrl_coordinateMode) override
    {
        return state.available;
    }
    bool getTouch(unsigned int, yarp::sig::Vector&) override
    {
        return state.available;
    }
};

// Model an interface reachable through a foreign device's IInterfaceView.
class ForeignJoypad final : public dinrail::IDevice, public dinrail::IInterfaceView
{
public:
    YarpJoypad source;
    bool open(const dinrail::Parameters&) override
    {
        return true;
    }
    bool close() override
    {
        return true;
    }
    void* viewInterface(const std::type_info& type) override
    {
        return type == typeid(yarp::dev::IJoypadController)
                   ? static_cast<yarp::dev::IJoypadController*>(&source)
                   : nullptr;
    }
};

template <class Interface> void checkReads(Interface& adapter, JoypadState& state)
{
    unsigned int count = 0;
    REQUIRE(adapter.getAxisCount(count));
    REQUIRE(count == 2);
    REQUIRE(adapter.getButtonCount(count));
    REQUIRE(count == 3);
    REQUIRE(adapter.getHatCount(count));
    REQUIRE(count == 1);
    double axis = 0;
    float button = 0;
    unsigned char hat = 0;
    REQUIRE(adapter.getAxis(1, axis));
    REQUIRE(axis == -0.75);
    REQUIRE(adapter.getButton(1, button));
    REQUIRE(button == 1.0f);
    REQUIRE(adapter.getHat(0, hat));
    REQUIRE(hat == 4);
    REQUIRE_FALSE(adapter.getAxis(2, axis));
    REQUIRE_FALSE(adapter.getButton(3, button));
    REQUIRE_FALSE(adapter.getHat(1, hat));

    state.axes[1] = 0.5;
    state.buttons[1] = 0.0f;
    state.hats[0] = 8;
    REQUIRE(adapter.getAxis(1, axis));
    REQUIRE(axis == 0.5);
    REQUIRE(adapter.getButton(1, button));
    REQUIRE(button == 0.0f);
    REQUIRE(adapter.getHat(0, hat));
    REQUIRE(hat == 8);

    state.available = false;
    REQUIRE_FALSE(adapter.getAxisCount(count));
    REQUIRE_FALSE(adapter.getButtonCount(count));
    REQUIRE_FALSE(adapter.getHatCount(count));
    REQUIRE_FALSE(adapter.getAxis(0, axis));
    REQUIRE_FALSE(adapter.getButton(0, button));
    REQUIRE_FALSE(adapter.getHat(0, hat));
}
} // namespace

TEST_CASE("Native joypad input is available through the registered YARP adapter")
{
    NativeJoypad device;
    dinrail::InterfaceAdapterRegistry registry;
    dinrail::YarpInteropPlugin{}.registerInterfaceAdapters(registry);
    auto adapter = registry.create(device, typeid(yarp::dev::IJoypadController));
    REQUIRE(adapter != nullptr);
    auto* joypad = static_cast<yarp::dev::IJoypadController*>(adapter->getInterface());
    REQUIRE(joypad != nullptr);
    checkReads(*joypad, device.state);

    unsigned int count = 0;
    yarp::sig::Vector vector;
    REQUIRE_FALSE(joypad->getTrackballCount(count));
    REQUIRE_FALSE(joypad->getTouchSurfaceCount(count));
    REQUIRE_FALSE(joypad->getStickCount(count));
    REQUIRE_FALSE(joypad->getStickDoF(0, count));
    REQUIRE_FALSE(joypad->getTrackball(0, vector));
    REQUIRE_FALSE(joypad->getTouch(0, vector));
    REQUIRE_FALSE(joypad->getStick(0, vector, yarp::dev::IJoypadController::JypCtrlcoord_POLAR));
    REQUIRE_FALSE(
        joypad->getStick(0, vector, yarp::dev::IJoypadController::JypCtrlcoord_CARTESIAN));
    REQUIRE_FALSE(joypad->eventDriven(true));
    REQUIRE_FALSE(joypad->isEventDriven());
}

TEST_CASE("Foreign YARP joypad input is available through the registered dinrail adapter")
{
    ForeignJoypad device;
    dinrail::InterfaceAdapterRegistry registry;
    dinrail::YarpInteropPlugin{}.registerInterfaceAdapters(registry);
    auto adapter = registry.create(device, typeid(dinrail::IJoypadControl));
    REQUIRE(adapter != nullptr);
    auto* joypad = static_cast<dinrail::IJoypadControl*>(adapter->getInterface());
    REQUIRE(joypad != nullptr);
    checkReads(*joypad, device.source.state);

    REQUIRE_FALSE(joypad->reconnect());
    dinrail::JoypadDeviceEvent event = dinrail::JoypadDeviceEvent::Disconnected;
    REQUIRE(joypad->getLastEvent(event));
    REQUIRE(event == dinrail::JoypadDeviceEvent::NoEvent);
}

TEST_CASE("Joypad adapters require the corresponding source interface")
{
    NativeJoypad native;
    ForeignJoypad foreign;
    dinrail::InterfaceAdapterRegistry registry;
    dinrail::YarpInteropPlugin{}.registerInterfaceAdapters(registry);
    REQUIRE(registry.create(native, typeid(dinrail::IJoypadControl)) == nullptr);
    REQUIRE(registry.create(foreign, typeid(yarp::dev::IJoypadController)) == nullptr);
}
