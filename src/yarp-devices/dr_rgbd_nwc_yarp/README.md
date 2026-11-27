# dr_rgbd_nwc_yarp

Network wrapper client of an RGBD sensor, exposing `dinrail::IRGBDSensor`. It connects to the ports of `dr_rgbd_nws_yarp`, or of the YARP `rgbdSensor_nws_yarp`, which use the same protocol.

The images are received by port callbacks, which store the most recent ones: the getters never block, and return the same images, with the same stamp sequence number, until new ones are received. The intrinsic and extrinsic parameters and the clip planes are read through the RPC port when the device is opened, so that the getters do not communicate with the server. `getImages()` returns the most recent color and depth images with the same sequence number.

The color stream is optional: if `remote_image_port` is not set only the depth is received, and the getters of the color image return `StatusCode::FailedPrecondition`.

| Parameter | Type | Units | Default | Required | Description |
|:---:|:---:|:---:|:---:|:---:|:---:|
| `remote_depth_port` | string | - | - | Yes | Depth port of the server, e.g. `/depthCamera/depthImage:o`. |
| `local_depth_port` | string | - | - | Yes | Port receiving the depth images. |
| `remote_rpc_port` | string | - | - | Yes | RPC port of the server, e.g. `/depthCamera/rpc:i`. |
| `local_rpc_port` | string | - | - | Yes | Local RPC port. |
| `remote_image_port` | string | - | - | No | Color port of the server, e.g. `/depthCamera/rgbImage:o`. |
| `local_image_port` | string | - | - | If `remote_image_port` is set | Port receiving the color images. |
| `depth_carrier` | string | - | tcp | No | Carrier of the depth connection. |
| `image_carrier` | string | - | tcp | No | Carrier of the color connection. |
| `frame_timeout` | double | s | 1.0 | No | Time without depth images after which the status is `Timeout`. |

The device can be opened through `dinrail::Device`, which loads it with the YARP interop plugin:

~~~cpp
dinrail::Parameters options;
options.put("device", "dr_rgbd_nwc_yarp");
options.put("remote_depth_port", "/depthCamera/depthImage:o");
options.put("local_depth_port", "/myApp/depthImage:i");
options.put("remote_rpc_port", "/depthCamera/rpc:i");
options.put("local_rpc_port", "/myApp/rpc:o");

dinrail::Device device;
dinrail::IRGBDSensor* sensor = nullptr;
if (device.open(options) && device.view(sensor))
{
    dinrail::DepthImage depth;
    sensor->getDepthImage(depth); // never blocks
}
~~~
