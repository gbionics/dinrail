This is the repo for dinrail, a library to encapsulate distint robot functionality in standalone C++ classes called "dinrail devices", that expose their functionalities over C++ interfaces. 

dinrail is meant as a drop-in replacement for the "device" part of YARP (Yet Another Robot Platform) library.

dinrail is a lightway C++ project that can also be integrated via FetchContent.

All optional functionalities (such as drop-in support for YARP device drivers) can be loaded at runtime thanks to `sharedlibpp`-powered plugins.

Dependencies of dinrail include:
* https://github.com/robotology/ycm-cmake-modules
* https://github.com/gbionics/sharedlibpp

For the CMake and pixi infrastructure, the structure of dinrail is inspired from:
* https://github.com/gbionics/idyntree
* https://github.com/robotology/how-to-export-cpp-library

For how the optional integration of YARP functionality is structured, the inspiration is provided by:
* https://github.com/gbionics/qpsolvers-eigen

For the actual functionality of dinrail devices, the main inspiration is the "device" part of YARP:
* https://github.com/robotology/yarp
