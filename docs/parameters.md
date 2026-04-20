
# `dinrail::Parameters` 

This page documents the basic usage pattern of `dinrail::Parameters` and the related typed value container `dinrail::Value`.

## Overview

`dinrail::Parameters` is a hierarchical key-value container, that can contain:

- scalar values (`bool`, `int`, `double`, `std::string`, `std::chrono::nanoseconds`)
- vector values (`std::vector<bool|int|double|std::string|std::chrono::nanoseconds>`)
- nested groups (`Parameters` objects)

`dinrail::Value` is the variant-backed value type returned by `dinrail::Parameters::find()` methods.

## Basic Example

```cpp
#include <dinrail/Parameters.h>

int main()
{
    dinrail::Parameters params;

    // Set scalar values.
    params.put("name", "left_arm");
    params.put("joints", 7);
    params.put("enabled", true);

    // Set vector
    const std::vector<double> gains{10.0, 10.0, 8.0};
    params.put("gains", gains);

    // Create a nested group.
    auto& limits = params.addGroup("limits");
    limits.put("max_velocity", 1.5);

    // Read with find() + Value conversion.
    const std::string name = params.find("name").asString();

    // Read with BLF-style getter.
    int joints = 0;
    if (params.getParameter("joints", joints))
    {
        // use joints
    }

    // Access a nested group.
    const auto& limitsGroup = params.findGroup("limits");
    if (!limitsGroup.isNull())
    {
        double maxVelocity = 0.0;
        limitsGroup.getParameter("max_velocity", maxVelocity);
    }

    return 0;
}
```

## `getParameter` vs `find` methods

`dinrail::Parameters` supports two ways of retrieving parameters, that are:
* The `find` method, that returns a reference `dinrail::Value` object. This method is **only supported for scalar parameters**. This method is inspired on the similar API of the [`yarp::os::Property`](https://www.yarp.it/latest/classyarp_1_1os_1_1Property.html) class.
* The `getParameter` methods, that take in input a reference to the parameter to fill. This method is **supported for all parameters type**, and is the only supported one to get vector values. These methods are inspired from the [`BipedalLocomotion::ParametersHandler::IParametersHandler`](https://gbionics.github.io/bipedal-locomotion-framework/classBipedalLocomotion_1_1ParametersHandler_1_1IParametersHandler.html) interface.

## Read or write a vector to a `dinrail::Parameters`

Support for reading or writing arbitrary vectors is available in `dinrail::Parameters`. 

This is achieved via the `dinrail::VectorProxy` class, that acts as a lightweight non-owning wrapper of existing vector classes like `std::vector` or `Eigen::VectorXd`, but supporting also `resize`.

### Write vector parameters

```cpp
#include <dinrail/Parameters.h>
#include <Eigen/Core>

dinrail::Parameters params;

const std::vector<int> jointIds{0, 1, 2, 3, 4, 5};
params.put("joint_ids", jointIds);

const std::vector<std::string> jointNames{"shoulder", "elbow", "wrist"};
params.put("joint_names", jointNames);

Eigen::VectorXd gains(6);
gains << 20.0, 20.0, 18.0, 15.0, 12.0, 10.0;

params.put("gains", gains);
```

### Read vector parameters

```cpp
#include <Eigen/Core>

Eigen::VectorXd gains;

if (params.getParameter("gains", gains))
{
    // gains now contains the values stored in params as an Eigen::VectorXd,
    // with the correct size as dinrail::VectorProxy support resizing
}

// If instead we pass a non-resizable container, getParameter is successful only
// if the vector has the right size
std::array<double, 6> gains6d;
std::array<double, 3> gains3d;

// Returns true
params.getParameter("gains", gains6d);

// Returns false
params.getParameter("gains", gains3d);
```

### Notes

- `find("...")` is not suitable for vector retrieval. Use `getParameter(...)` for vectors.
- `getParameter(...)` returns `false` if the key is missing or the stored type does not match.
- The same pattern works for `int`, `double`, `std::string`, and `std::chrono::nanoseconds` vectors.

## Missing Keys and Missing Groups

- `find("missing")` returns a null `dinrail::Value` (`value.isNull() == true`).
- `findGroup("missing")` returns a null-sentinel group (`group.isNull() == true`).

This allows graceful checks without exceptions when a key/group is absent.

## Notes on `dinrail::Value`

`dinrail::Value` supports direct typed access via:

- `value.is<T>()`
- `value.as<T>()`

It also provides `yarp::os::Property`-like scalar helpers:

- `asBool()`
- `asInt()`
- `asDouble()`
- `asString()`

Use `is<T>()` / `as<T>()` when strict typing is needed. Use helper conversions only when compatibility-style behavior is desired, for example if you want to cast an `int` value to a `double`.
