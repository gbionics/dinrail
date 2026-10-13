# Publish a dinrail RGB camera through YARP

This example uses `dinrail-runner` to open a fake RGB camera, attach it to
YARP's `frameGrabber_nws_yarp`, and display its images with `yarpview`.
The two XML configurations use the same network wrapper and 640x480 RGB frames:

| Configuration | Camera device | Image output port |
| --- | --- | --- |
| [dr_camera_fake.xml](dr_camera_fake.xml) | dinrail `dr_camera_fake`, in `framegrabber` mode | `/dinrail-camera` |
| [fakeFrameGrabber.xml](fakeFrameGrabber.xml) | YARP `fakeFrameGrabber`, in `line` mode | `/yarp-camera` |

The `COLOR` capability selects RGB streaming. The wrapper publishes images at
its configured period of 0.033 seconds, about 30 Hz, and opens an RPC port at
`<image-output-port>/rpc`. Its startup action attaches the camera; its shutdown
action detaches it.

## Prepare the environment

Run this example in an environment in which both dinrail and YARP are installed,
with their binaries and plugins discoverable.

Run the commands from the repository root, in separate terminals using that
environment. Start one name server and keep it running for both examples:

```sh
yarpserver
```

## Run the dinrail camera

In a second terminal, start the camera and its publisher:

```sh
dinrail-runner --config examples/yarp-run-dinrail-camera-as-yarp-camera/dr_camera_fake.xml
```

In a third terminal, open the viewer:

```sh
yarpview --name /view/dinrail-camera
```

Once the viewer is open, connect the image stream from a fourth terminal:

```sh
yarp connect /dinrail-camera /view/dinrail-camera
```

The viewer shows a 640x480 television-style test chart with a circle, grid,
color bars, grayscale and resolution stripes, plus the machine's current local
clock with milliseconds. The example enables `test_pattern` in
`dr_camera_fake.xml`; this option defaults to `false` on the device. Set it to
`false` to supply frames through `IRGBDSensorSimulation` instead.

## Compare with YARP's fake camera

Keep the name server running. In another terminal, start the YARP configuration:

```sh
dinrail-runner --config examples/yarp-run-dinrail-camera-as-yarp-camera/fakeFrameGrabber.xml
```

Open a second viewer in its own terminal:

```sh
yarpview --name /view/yarp-camera
```

Connect it after the viewer is open:

```sh
yarp connect /yarp-camera /view/yarp-camera
```

YARP's `fakeFrameGrabber` generates a moving line pattern. Both examples can run
at the same time because they use distinct publisher and viewer port names.
The publishing and viewing commands follow the same procedure for both devices;
the camera device type, mode and port names are selected by their XML files.

Stop each `dinrail-runner` process with Ctrl+C, close the viewers, and stop
the name server when finished.
