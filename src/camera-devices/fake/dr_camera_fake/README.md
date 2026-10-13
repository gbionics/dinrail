# dr_camera_fake Parameters

This document describes the parameters accepted by the `dr_camera_fake` dinrail
device. One in-memory device supports both 2D framegrabbers and RGBD sensors,
driven through their simulation interfaces or an optional diagnostic test pattern.

## Parameters

| Group name | Parameter name | Type | Units | Default Value | Required | Description | Notes |
|:----------:|:---------------|:----:|:-----:|:-------------:|:--------:|:------------|:------|
| - | mode | string | - | `rgbd` | 0 | Camera acquisition mode | Accepts `framegrabber` or `rgbd`. |
| - | width | int | pixels | 640 | 0 | Width of RGB and raw frames | Must be greater than zero. |
| - | height | int | pixels | 480 | 0 | Height of RGB and raw frames | Must be greater than zero. |
| - | depth_width | int | pixels | 640 | 0 | Width of depth frames | Must be greater than zero; independent of RGB width. |
| - | depth_height | int | pixels | 480 | 0 | Height of depth frames | Must be greater than zero; independent of RGB height. |
| - | test_pattern | bool | - | `false` | 0 | Generate a diagnostic chart with the current local time | Replaces simulator-supplied RGB and raw pixels on acquisition. |

The device exposes camera controls, raw acquisition, RGBD acquisition, visual
parameters, precise timing and their simulation interfaces. Interfaces remain
viewable in both modes. In `framegrabber` mode, RGB and raw acquisition remain
available, while depth operations return `Unimplemented`. No acquisition thread
is used.

## Simulation

By default, frames start at zero and remain unchanged until a simulator supplies
new data. Use `IFrameGrabberImageRawSimulation::setImage` for monochrome data and
`IRGBDSensorSimulation::setImages` to atomically supply RGB/depth frames and their
acquisition stamps. Setters copy even non-contiguous matrices and reject incorrect
dimensions or types. Returned matrices are independent copies.

Mirroring applies to returned frames. Resolution changes reset the associated
frames and regenerate pinhole calibration; FOV changes regenerate calibration.
Depth accuracy and clip planes are reported metadata, without modifying supplied
measurements. Feature values are normalized to `[0, 1]`. Feature capabilities,
calibration, sensor status and error messages are configurable by simulation.
`setOnePush` acknowledges supported requests without changing simulator data.

## Test Pattern

Set `test_pattern=true` to generate a television-style diagnostic chart with a
circle, grid, RGB color bars, grayscale and resolution stripes. The chart also
displays the machine's current local date and time, including milliseconds.

Each RGB, raw or paired acquisition refreshes the chart and records a system-clock
timestamp and sequence number. Raw acquisition returns the grayscale version;
mirroring still applies. The next acquisition replaces any RGB or raw pixels
supplied through simulation setters. Disable this option for simulator-driven
frames and timestamps.

## Example Configuration (C++)

Configure a 2D RGB camera with a diagnostic test pattern through `dinrail::Parameters`:

```cpp
#include <dinrail/Parameters.h>

dinrail::Parameters opts;
opts.put("device", "dr_camera_fake");
opts.put("mode", "framegrabber");
opts.put("width", 640);
opts.put("height", 480);
opts.put("test_pattern", true);
```

For RGBD acquisition, use `mode=rgbd` and optionally configure `depth_width` and
`depth_height`.
