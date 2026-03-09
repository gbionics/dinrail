# dinrail

Experimental prototype drop-in replacement for "devices" part of YARP, used to run robots in Generative Robotics.

## Install and usage

The project is a fairly standard CMake project. Once it is installed, a basic example is:

~~~cmake
find_package(dinrail REQUIRED)
~~~

Basic usage of `dinrail::Parameters`:

~~~cpp
#include <dinrail/Parameters.h>

int main()
{
	dinrail::Parameters params;

	// Set scalar parameters.
	params.put("name", "left_arm");
	params.put("joints", 7);
	params.put("enabled", true);

	// Create and fill a nested group.
	auto& limits = params.addGroup("limits");
	limits.put("max_velocity", 1.5);

	// Read parameters back.
	const auto& name = params.find("name").asString();
	int joints = 0;
	params.getParameter("joints", joints);

	const auto& limitsGroup = params.findGroup("limits");
	double maxVelocity = 0.0;
	limitsGroup.getParameter("max_velocity", maxVelocity);

	return 0;
}
~~~

Detailed documentation for major class can be found in the following docs:
* [`dinrail::Parameters` and `dinrail::Value`](docs/parameters.md)

If you are migrating code from YARP devices, please check the relevant docs:
* [Migrating code from YARP devices to dinrail devices](docs/yarp-migration.md)

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