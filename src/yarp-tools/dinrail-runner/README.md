# dinrail-runner

The `dinrail-runner` is a command line tool that is useful to launch multiple YARP devices at once. 

At the moment, it is just a drop-in replacement for `yarprobotinterface` (without the hardcoded  `/yarprobotinterface` rpc port) that uses `libYARP_robotinterface` and is only able to run YARP devices (and not `dinrail` devices), but in the future will be evolved in a tool that will be able to run both YARP devices and dinrail device.

One of the main uses of the `dinrail-runner` (as it was of the `yarprobotinterface`)
main program to provide a network "interface", via Network Server Wrappers (NWS) devices, to a robot.

However, the `dinrail-runner` can be used to launch YARP devices of any kind.

At the moment details of the xml format of the files loaded by `dinrail-runner` are documented in YARP, see https://www.yarp.it/v3.12/group__yarp__robotinterface__xml__config__files.html .

## Command Line Parameters

`--config ./configdir/config.xml`
- Specify the path of the `.xml` file to load and that
  describes the YARP devices to launch.

`--portprefix portprefix`
- If specified, this values override the portprefix attribute
  of the robot element of the xml file.

`--verbose`:
- If this option is specified, enable verbose output of the xml parser.

`--dryrun`
- If this option is specified, then xml file is only loaded without actually opening devices.
  This option is useful to validate if xml files are well formed.

`--enable_tags (xxx yyy ... zzz)`
- This options can be used to enable optional devices which have been marked with the in `enabled_by` attribute in the xml file. See https://www.yarp.it/v3.12/group__yarp__robotinterface__xml__config__files.html .

`--disable_tags (xxx yyy ... zzz)`
- This options can be used to disable included devices which have been marked with the in `disabled_by` attribute in the xml file. See https://www.yarp.it/v3.12/group__yarp__robotinterface__xml__config__files.html .


`--autocloseAfterStart`
- This options automatically closes `dinrail-runner` after opening all requested devices. It can be used for debugging purposes to check the behavior of selected devices and
 if they properly close.

## Configuration Files

`dinrail-runner` loads the xml file from the location specified in the `--config` option.

## dinrail-runner-rt

The CMake infrastructure install two identical binaries, `dinrail-runner` and `dinrail-runner-rt` . The two identical commands are meant to be used in a context you want to run YARP devices in a priligied context and with elevated permissions, for example as you want to run them with "real time" (i.e. `-rt`) permissions.

In case you need to give elevated permission to an executable, you can use the `dinrail-runner-rt`, while keeping the default permission for `dinrail-runner`.
