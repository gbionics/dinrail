# dr_rgbd_nws_yarp

Network wrapper server of a `dinrail::IRGBDSensor`. It publishes the images on YARP ports and serves the parameters of the sensor on an RPC port, with the protocol of the YARP `rgbdSensor_nws_yarp`, so that both `dr_rgbd_nwc_yarp` and the standard YARP clients (`RGBDSensorClient`, `yarpview`, ...) can be connected to it.

The sensor is either the dinrail device named by `subdevice`, opened with the other parameters of the configuration, or a YARP device attached with the `attach` action. The attached device can wrap a dinrail device implementing `dinrail::IRGBDSensor` (as `dr_device_yarp`, whose native interfaces are used directly), implement `dinrail::IRGBDSensor`, or implement `yarp::dev::IRGBDSensor`, used through an adapter.

A periodic thread checks the stamp of the latest images with the non-blocking `dinrail::IRGBDSensor::getImagesStamp()`, reads them with `getImages()` only when their sequence number changes (sensors that do not implement `getImagesStamp()` are read at every iteration) and publishes them, with their stamps as envelopes. For sensors that stream only depth images (`getImages()` returns `FailedPrecondition`) only the depth is published. As the getters never wait for the camera, `period` only bounds the latency added by the wrapper and does not need to match the framerate of the camera. Images are published only on the ports with at least one connection.

| Parameter | Type | Units | Default | Required | Description |
|:---:|:---:|:---:|:---:|:---:|:---:|
| `name` | string | - | - | Yes | Prefix of the ports: `<name>/rgbImage:o`, `<name>/depthImage:o` and `<name>/rpc:i`. Must start with `/`. |
| `period` | double | s | 0.005 | No | Period of the thread checking for new images. |
| `subdevice` | string | - | - | No | dinrail device to open. If missing, the sensor must be attached. |

Example, with the RealSense dinrail device of gb-dinrail-device-realsense2 (see `dr_device_yarp` to also publish its IMU):

~~~xml
<device name="depthCamera" type="dr_rgbd_nws_yarp">
  <param name="name"> /depthCamera </param>
  <param name="subdevice"> realsense </param>
  <param name="depth_resolution"> (480 270) </param>
  <param name="framerate"> 30 </param>
</device>
~~~

Differently from `rgbdSensor_nws_yarp`, the extrinsic matrix is sent as a single element of the RPC response, which is the format expected by `RGBDSensorClient`.
