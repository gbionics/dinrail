# Migrating Code from YARP Devices to dinrail Devices


## From `yarp::os::Property` to `dinrail::Parameters`

### Scalar values

As long as you are retrieving scalar parameters, you can use similar APIs:

- `yarp::os::Property` -> `dinrail::Parameters`
- `property.find("k")` -> `params.find("k")`
- `property.check("k")` -> `params.check("k")`
- `property.findGroup("G")` -> `params.findGroup("G")`
- `property.put("k", v)` -> `params.put("k", v)` or `params.setParameter("k", v)`

## Nested groups

Nested groups are handled with:

- `params.addGroup("GROUP")`
- `params.findGroup("GROUP")`

## Vector values

If instead you are reading and writing vector values, you must port your code to use `setParameter(...)` and `getParameter(...)` with `dinrail::VectorProxy`.

Unlike scalar values, vector values are not accessed via `find("k")` returning a `dinrail::Value`. The intended API is:

- write with `params.setParameter("k", container)`
- read with `params.getParameter("k", container)`

For example, code such as:

```cpp
yarp::os::Bottle* gains = property.find("gains").asList();
std::vector<double> values;

for (int i = 0; i < gains->size(); ++i)
{
	values.push_back(gains->get(i).asFloat64());
}
```

can be migrated to:

```cpp
#include <dinrail/Parameters.h>

std::vector<double> values;

const bool ok = params.getParameter("gains", values);
```

Similarly, writing a vector becomes:

```cpp
std::vector<std::string> jointNames{"shoulder", "elbow", "wrist"};

params.setParameter("joint_names", jointNames);
```

If your destination container is resizable, `getParameter(...)` resizes it as needed. If it is not resizable, such as `std::array`, the call succeeds only if the stored vector size already matches the destination size.

For more details and more examples, see [parameters.md](parameters.md).

Note that while vector values of heterogenous types are supported in `yarp::os::Property`, to keep the complexity low these are not supported in `dinrail::Parameters`.

