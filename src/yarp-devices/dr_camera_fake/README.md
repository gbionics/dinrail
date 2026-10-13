# dr_camera_fake YARP wrapper

The wrapper opens the native `dr_camera_fake` plugin and exposes
`IFrameGrabberControls`, `IFrameGrabberImage`, `IFrameGrabberImageRaw`,
`IRGBDSensor` (including visual parameters) and `IPreciselyTimed`.
Install both the native and YARP plugins. No YARP name server is needed to open
this device locally.

Example configuration for a 2D framegrabber:

```ini
device dr_camera_fake
mode framegrabber
width 640
height 480
```

For RGBD acquisition use `mode rgbd` and optionally specify `depth_width` and
`depth_height`. Both modes use the same device. Simulation data is supplied
through the native device's simulation interfaces; a C++ wrapper owner can
access these with `wrapper.device().view(...)`.
