# `dinrail::ParametersFinder`

A common requirement for devices or in general software that reads configuration from `dinrail::Paramters` is to be able to load this parameters from a configuration file available in the filesystem, eventually overriding its content with parameters specified from the command line.

To provide these functionality, `dinrail` provides the `dinrail::ParametersFinder` class, that finds parameters 

## Basic example

~~~cxx
#include <dinrail/Parameters.h>

int main(int argc, char *argv[])
{
    dinrail::ParametersFinder paramsFinder;

    // Set the subdirectory of share in which
    // the project installs its configuration file, useful
    // for dinrail::ParametersFinder to solve file names that
    // do not specified with a `package://` URIs
    // This can be overriden by a --package-name setting in argc/argv
    paramsFinder.setDefaultPackageName("example_package")

    // Set the default config file, can be overriden by
    // a --config setting in the argc/argv
    paramsFinder.setDefaultConfigFile("dinrail_runner.toml");

    // Pass command line arguments, and actually search for files
    paramsFinder.configure(argc, argv);

    // Access the parameters
    int joints = 0;
    if (paramsFinder.getParams().getParameter("joints", joints))
    {
        // use joints
    }

    // The dinrail::ParametersFinder can also be used to find arbitrary
    // non-parameters files, using package:// or the dinrail::ParametersFinder
    // search policy
    std::string example_file_path = paramsFinder.findFile("package://example_package/example_file.txt");
    std::string example_image_path = paramsFinder.findFile("example_image.png");
}
~~~

## Search policy

By default, the `dinrail::ParametersFinder` loads its parameters from the file specified by `setDefaultConfigFile` or the one specified via the `--config` parameter passed in the `argc, argv` pair. In case the specified file is specified via 
a `package://` URI, the file is searched following the logic described in https://github.com/gbionics/resolve-robotics-uri-cpp/blob/main/rru_spec.md .

If the file name does not start with `package://`, it is searched on base of the available `robot-name` and `package-name`, that are determined as in the following:
* `robot-name`: 
  * If `--robot-name` is specified in the command line arguments, it has its value, otherwise
  * if `DINRAIL_ROBOT_NAME` environment variable is defined, it has its value, otherwise
  * if `YARP_ROBOT_NAME` environment variable is defined, it has its value, otherwise
  * the `robot-name` does not any value, and the robot-specific directories are not searched for.

* `package-name`:
  * If `--package-name` is specified in the command line arguments, it has its value, otherwise
  * if `setDefaultPackageName` is specified, use that value
  * If package name is not specified, passing a file without `package://` fails

Once the `robot-name` and `package-name` are determined, the file `<file_relative_path>` are searched as in:

* If `robot-name` is specified, first look for `package://<package-name>/robots/<robot-name>/<file_relative_path>`,
*  if no file is found in the previous step, look for `package://<package-name>/<file_relative_path>`.
