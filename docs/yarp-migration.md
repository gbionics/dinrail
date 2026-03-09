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

If instead you are reading and writing vector values, you must port your code to use the `setParameters` and `getParameters`, see:



Note that while vector values of heterogenous types are supported in `yarp::os::Property`, to keep the complexity low these are not supported in `dinrail::Parameters`.