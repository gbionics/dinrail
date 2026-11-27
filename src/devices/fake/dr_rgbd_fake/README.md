# dr_rgbd_fake

Fake RGBD sensor implementing `dinrail::IRGBDSensor` and `dinrail::IRGBDSensorSimulation`.

When opened, it provides an RGB8 color image filled with 128 and a depth image filled with `depth`. The images, which must keep the size set when opened, and the status can be changed through `IRGBDSensorSimulation`. If `framerate` is positive, the sequence number of the images is incremented, and their time updated, `framerate` times per second since the device was opened, as by a streaming camera.

| Parameter | Type | Units | Default | Description |
|:---:|:---:|:---:|:---:|:---:|
| `width` | int | pixel | 640 | Width of the images. |
| `height` | int | pixel | 480 | Height of the images. |
| `focal_length` | double | pixel | 500.0 | Focal length; the principal point is the center of the image. |
| `depth` | double | m | 1.0 | Value of all the pixels of the depth image. |
| `framerate` | double | Hz | 0.0 | Streaming rate, 0 for static images. |
| `enable_rgb` | bool | - | true | If false, the sensor streams only depth images, and the methods involving the color images return `FailedPrecondition`. |
