# dinrail

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