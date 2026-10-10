# dinrail

<img alt="Image" src="https://github.com/user-attachments/assets/062cdb97-7b0c-4955-b2de-55a16ff5ec2a" />


Almost drop-in replacement for "devices" part of YARP, used to run robots in Generative Robotics.

The main goal is to implement an almost drop-in replacement for the "devices" part of YARP, with additional planner properties and features:
* lightweight (no heavy dependencies for the code part)
* compact (easy to mantain and extend)
* with support for writing devices in non-C++ languages, like Python

To achieve easy transition, the goal of `dinrail` is to automatically support opening `YARP` devices as `dinrail` devices, and viceversa.

## Install and usage

The project is a fairly standard CMake project. Once it is installed, a basic example is:

~~~cmake
find_package(dinrail REQUIRED)

target_link_libraries(target PRIVATE dinrail::dinrail)
~~~

## Basic API

Configure and open a fake device, then query it through a typed interface:

~~~cpp
#include <dinrail/Device.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/Parameters.h>

dinrail::Parameters opts;
opts.put("device", "dr_controlboard_fake");
opts.put("number_of_joints", 3);
opts.put("joint_names", std::vector<std::string>{"shoulder", "elbow", "wrist"});
opts.put("joint_type", std::vector<std::string>{"revolute", "prismatic", "revolute"});

dinrail::Device device;
device.open(opts);

dinrail::IAxisInfo* axisInfo = nullptr;
device.view(axisInfo);

int axes = 0;
axisInfo->getAxes(&axes); // axes == 3

std::string name;
axisInfo->getAxisName(0, name); // name == "shoulder"

device.close();
~~~

### Basic CLI

To know which devices are available in your install, just run:

~~~
dinrail dev --list
~~~

to see the available devices.

## Additional documentation

See the following links for more documentation on the dinrail project:
* [dinrail::Parameters](docs/parameters.md) — how to use the hierarchical key-value configuration container
* [Plugin discovery](docs/plugin-discovery.md) — how device and interop plugins are discovered at runtime
* [Interoperability plugins](docs/interop-plugins.md) — how to write plugins for foreign device systems
* [YARP migration guide](docs/yarp-migration.md) — guides for migrating parameters, logging, devices, and sensor adapters
* [Convention and Guidelines](docs/conventions-and-guidelines.md) - convention and guidelines for dinrail code

## Examples

Examples documentation (including Pixi commands) is available at:

* [examples/README.md](examples/README.md)

## Built-in device documentation

These `dinrail` devices are part of the `dinrail-devices` package.

* [dr_battery_fake README](src/devices/fake/dr_battery_fake/README.md) — parameters accepted by the `dr_battery_fake` plugin
* [dr_controlboard_fake README](src/devices/fake/dr_controlboard_fake/README.md) — parameters accepted by the `dr_controlboard_fake` plugin
* [dr_multiplenalogsensors_fake README](src/devices/fake/dr_multiplenalogsensors_fake/README.md) — parameters accepted by the `dr_multiplenalogsensors_fake` plugin

### YARP network devices

All bult-in devices in `dinrail-devices` are also available as `yarp` devices with the same names and parameters, inside the `dinrail-yarp-devices` package.

Furthermore, in `dinrail-yarp-devices` the following YARP-only devices are available:

* [dr_controlboard_nwc_yarp README](src/yarp-devices/dr_controlboard_nwc_yarp/README.md) — parameters for connecting to a remote control board over the YARP network
* [dr_controlboard_nws_yarp README](src/yarp-devices/dr_controlboard_nws_yarp/README.md) — parameters for exposing an attached control board over the YARP network

## Versioning policy

Any ABI, API or network protocol incompatible change will result in a minor (at least) release bump.

## Development

For internal developement, this repos uses [`pixi`](https://pixi.prefix.dev/latest/).

Whenever you work on it, run:

~~~
pixi run test
~~~

to run the test suite and:

~~~
pixi run format
~~~

to format the source code.
