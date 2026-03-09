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
#include <dinrail/GenericVector.h>

int main()
{
    dinrail::Parameters params;

    // Set scalar values.
    params.put("name", "left_arm");
    params.put("joints", 7);
    params.put("enabled", true);

    // Set vector values via GenericVector view.
    const std::vector<double> gains{10.0, 10.0, 8.0};
    params.put("gains", dinrail::GenericVector<const double>::Ref(gains));

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

## `getParameters` vs `find` methods

`dinrail::Parameters` supports two ways of retrieving parameters, that are:
* The `find` method, that returns a reference `dinrail::Value` object. This method is **only supported for scalar parameters**. This method is inspired on the similar API of the [`yarp::os::Property`](https://www.yarp.it/latest/classyarp_1_1os_1_1Property.html) class.
* The `getParameter` methods, that take in input a reference to the method to fill. This method is **supported for all parameters type**, and is the only supported one to get vector values. These methods are inspired from the [`BipedalLocomotion::ParametersHandler::IParametersHandler`](https://gbionics.github.io/bipedal-locomotion-framework/classBipedalLocomotion_1_1ParametersHandler_1_1IParametersHandler.html) interface.

## Read or write a vector to a `dinrail::Parameters`

Support for reading or writing arbitrary vectors is available in `dinrail::Parameters`. 

This is achieved via the `dinrail::GenericVector` class, that acts as a lightweight non-owning wrapper of existing vector classes like `std::vector` or `Eigen::VectorXd`, but supporting also `resize`.

### Write vector parameters

```cpp
#include <dinrail/Parameters.h>
#include <dinrail/GenericVector.h>
#include <Eigen/Core>

dinrail::Parameters params;

const std::vector<int> jointIds{0, 1, 2, 3, 4, 5};
params.put("joint_ids", dinrail::GenericVector<const int>::Ref(jointIds));

const std::vector<std::string> jointNames{"shoulder", "elbow", "wrist"};
params.put("joint_names", dinrail::GenericVector<const std::string>::Ref(jointNames));

Eigen::VectorXd gains(6);
gains << 20.0, 20.0, 18.0, 15.0, 12.0, 10.0;

dinrail::GenericVector<const double> gainsView(
    std::span<const double>(gains.data(), static_cast<std::size_t>(gains.size())));
params.put("gains", dinrail::GenericVector<const double>::Ref(gainsView));
```

For non-`bool` vectors, `put(...)` expects a `GenericVector<const ...>::Ref`.
For `Eigen::VectorXd`, create a `GenericVector` view around its memory and pass it to `put(...)`.

### Read vector parameters

```cpp
#include <dinrail/GenericVector.h>
#include <Eigen/Core>

Eigen::VectorXd gains;
auto resizeLambda = [&gains](std::size_t newSize) -> std::span<double> {
    gains.conservativeResize(static_cast<Eigen::Index>(newSize));
    return std::span<double>(gains.data(), static_cast<std::size_t>(gains.size()));
};

dinrail::GenericVector<double> gainsView(
    std::span<double>(gains.data(), static_cast<std::size_t>(gains.size())),
    resizeLambda);

if (params.getParameter("gains", dinrail::GenericVector<double>::Ref(gainsView)))
{
    // gains now contains the values stored in params as an Eigen::VectorXd
}
```

When using Eigen dynamic vectors, the resize lambda is required so `getParameter()` can resize the destination.

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
