# dinrail-camera

Camera interfaces live in `dinrail::dinrail-camera`, which links OpenCV core.
`dinrail::dinrail` does not depend on OpenCV. Enable camera support with
`-DDINRAIL_USES_OPENCV=ON`; YARP support also enables the camera module.
This directory can be built independently against an installed dinrail.

Consumers use `find_package(dinrail-camera REQUIRED)` and link
`dinrail::dinrail-camera`. Images are owned `cv::Mat` values: RGB uses
`CV_8UC3` with RGB channel order, raw images use `CV_8UC1`, and depth uses
`CV_32FC1` with distances in meters. Implementations copy simulation inputs
and return independent output storage, including for non-contiguous inputs.
Acquisition stamps use `dinrail::Stamp` (nanoseconds and sequence number).
Calibration uses `dinrail::Parameters` and a 4x4 `CV_64FC1` extrinsic transform.

Simulation interfaces accompany controls, raw acquisition and RGBD acquisition.
They let simulators supply frames, timestamps, calibration, capabilities and errors.
