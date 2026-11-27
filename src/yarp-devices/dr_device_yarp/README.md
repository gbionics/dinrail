# dr_device_yarp

YARP device that opens any dinrail device, so that YARP network wrappers can be attached to it.

- The dinrail-aware wrappers, such as `dr_rgbd_nws_yarp`, use the native interfaces of the wrapped device through `dinrail::IWrappedDinrailDevice`, without converting the data to the YARP types.
- The inertial sensors of the wrapped device (gyroscopes, linear and angular accelerometers, magnetometers and orientation sensors) are exposed through the YARP multiple analog sensors interfaces, so that they can be published by `multipleanalogsensorsserver`. The ones the wrapped device does not implement are exposed without sensors.

| Parameter | Type | Units | Default | Required | Description |
|:---:|:---:|:---:|:---:|:---:|:---:|
| `subdevice` | string | - | - | Yes | dinrail device to open. The other parameters of the configuration are passed to it. |

Example, publishing the images and the IMU of the RealSense dinrail device of gb-dinrail-device-realsense2:

~~~xml
<device name="realsense" type="dr_device_yarp">
  <param name="subdevice"> realsense </param>
  <param name="depth_resolution"> (480 270) </param>
  <param name="framerate"> 30 </param>
</device>

<device name="depthCamera" type="dr_rgbd_nws_yarp">
  <param name="name"> /depthCamera </param>
  <action phase="startup" level="5" type="attach">
    <paramlist name="networks">
      <elem name="realsense"> realsense </elem>
    </paramlist>
  </action>
  <action phase="shutdown" level="5" type="detach" />
</device>

<device name="depthCamera-inertials" type="multipleanalogsensorsserver">
  <param name="name"> /depthCamera/inertials </param>
  <param name="period"> 5 </param>
  <action phase="startup" level="5" type="attach">
    <paramlist name="networks">
      <elem name="realsense"> realsense </elem>
    </paramlist>
  </action>
  <action phase="shutdown" level="5" type="detach" />
</device>
~~~
