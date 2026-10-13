// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause
#include "../../camera-devices/fake/dr_camera_fake/DrCameraFake.h"
#include <catch2/catch_test_macros.hpp>
#include <dinrail/Device.h>
#include <dinrail/YarpCameraAdapters.h>
#include <dinrail/YarpStampConverter.h>

TEST_CASE("Camera adapters preserve pixels, geometry and stamps in both directions")
{
    // Create a native camera as the source for the adapters.
    dinrail::DrCameraFake fake;
    dinrail::Parameters config;
    config.put("width", 3);
    config.put("height", 2);
    config.put("depth_width", 3);
    config.put("depth_height", 2);
    REQUIRE(fake.open(config));

    // Provide distinct RGB channels, depth values and an acquisition stamp.
    cv::Mat rgb(2, 3, CV_8UC3, cv::Scalar(7, 33, 99));
    cv::Mat depth(2, 3, CV_32FC1, cv::Scalar(2.25));
    dinrail::Stamp stamp{std::chrono::nanoseconds(123456789), 5};
    REQUIRE(fake.setImages(rgb, depth, stamp, stamp).ok());

    // Adapt to YARP and back to verify both conversion directions.
    dinrail::InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor> toYarp(fake);
    dinrail::InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor> toNative(toYarp);
    cv::Mat outRgb, outDepth;
    dinrail::Stamp rs, ds;
    REQUIRE(toNative.getImages(outRgb, outDepth, &rs, &ds).ok());
    REQUIRE(outRgb.at<cv::Vec3b>(0, 2) == cv::Vec3b(7, 33, 99));
    REQUIRE(outDepth.at<float>(1, 2) == 2.25f);
    REQUIRE(rs.time == stamp.time);
    REQUIRE(ds.sequenceNumber == 5);

    // Configuration setters must reach the native camera through both adapters.
    REQUIRE(toNative.setDepthClipPlanes(0.2, 8).ok());
    double near, far;
    REQUIRE(fake.getDepthClipPlanes(near, far).ok());
    REQUIRE(far == 8);

    // Calibration, extrinsics and supported encodings must survive conversion.
    dinrail::Parameters intrinsic;
    REQUIRE(toNative.getRgbIntrinsicParam(intrinsic).ok());
    REQUIRE(intrinsic.findGroup("cameraDistortionModelGroup").check("k1"));
    cv::Mat extrinsic;
    REQUIRE(toNative.getExtrinsicParam(extrinsic).ok());
    REQUIRE(extrinsic.at<double>(3, 3) == 1);
    std::vector<dinrail::CameraConfig> configurations;
    REQUIRE(toNative.getRgbSupportedConfigurations(configurations).ok());
    REQUIRE(configurations[0].pixelCoding == dinrail::PixelCoding::RGB);

    // YARP pointer outputs are checked, and feature settings round-trip.
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>
        controls(fake);
    REQUIRE_FALSE(controls.getFeature(0, nullptr));
    REQUIRE_FALSE(controls.getCameraDescription(nullptr));
    dinrail::InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>
        nativeControls(controls);
    REQUIRE(nativeControls.setMode(0, dinrail::FeatureMode::Auto).ok());
    dinrail::FeatureMode mode;
    REQUIRE(nativeControls.getMode(0, mode).ok());
    REQUIRE(mode == dinrail::FeatureMode::Auto);
    REQUIRE(nativeControls.setFeature(0, 0.25, 0.75).ok());
    double a, b;
    REQUIRE(nativeControls.getFeature(0, a, b).ok());
    REQUIRE(a == 0.25);
    REQUIRE(b == 0.75);

    // Raw frames also round-trip through the YARP monochrome image type.
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw> raw(
        fake);
    dinrail::InterfaceAdapter<dinrail::IFrameGrabberImageRaw, yarp::dev::IFrameGrabberImageRaw>
        nativeRaw(raw);
    REQUIRE(fake.setImage(cv::Mat(2, 3, CV_8UC1, cv::Scalar(42))).ok());
    cv::Mat mono;
    REQUIRE(nativeRaw.getImage(mono).ok());
    REQUIRE(mono.at<unsigned char>(1, 2) == 42);

    // The inherited YARP crop implementation works with the raw adapter.
    yarp::sig::ImageOf<yarp::sig::PixelMono> crop;
    yarp::sig::VectorOf<std::pair<int, int>> vertices;
    vertices.push_back({0, 0});
    vertices.push_back({1, 1});
    REQUIRE(raw.getImageCrop(YARP_CROP_RECT, vertices, crop));
    REQUIRE(crop.width() == 2);
}

// A YARP fixture supplies BGR, mono or unsupported pixel encodings.
class BgrCamera : public dinrail::InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>
{
public:
    using InterfaceAdapter::InterfaceAdapter;
    int coding{VOCAB_PIXEL_BGR};
    bool getRgbImage(yarp::sig::FlexImage& image, yarp::os::Stamp* stamp) override
    {
        image.setPixelCode(coding);
        image.resize(3, 2);

        // Write each row separately to account for YARP image padding.
        for (int r = 0; r < 2; ++r)
        {
            for (int c = 0; c < 3; ++c)
            {
                auto* pixel
                    = image.getRawImage() + r * image.getRowSize() + c * image.getPixelSize();
                pixel[0] = 9;
                if (image.getPixelSize() >= 3)
                {
                    pixel[1] = 17;
                    pixel[2] = 25;
                }
            }
        }
        if (stamp)
        {
            *stamp = yarp::os::Stamp(4, 0.125);
        }
        return true;
    }
};

TEST_CASE("YARP BGR and mono images convert to RGB and unsupported coding leaves outputs untouched")
{
    dinrail::DrCameraFake fake;
    REQUIRE(fake.open({}));
    BgrCamera camera(fake);
    dinrail::InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor> adapter(camera);

    // BGR channel order is reversed and acquisition time is converted to nanoseconds.
    cv::Mat rgb;
    dinrail::Stamp stamp;
    REQUIRE(adapter.getRgbImage(rgb, &stamp).ok());
    REQUIRE(rgb.at<cv::Vec3b>(1, 2) == cv::Vec3b(25, 17, 9));
    REQUIRE(stamp.time == std::chrono::nanoseconds(125000000));

    // A monochrome input becomes an RGB image with three equal channels.
    camera.coding = VOCAB_PIXEL_MONO;
    REQUIRE(adapter.getRgbImage(rgb).ok());
    REQUIRE(rgb.at<cv::Vec3b>(0, 0) == cv::Vec3b(9, 9, 9));

    // Unsupported encodings must preserve the previous output image.
    camera.coding = VOCAB_PIXEL_RGBA;
    REQUIRE(adapter.getRgbImage(rgb).code() == dinrail::StatusCode::Unimplemented);
    REQUIRE(rgb.at<cv::Vec3b>(0, 0) == cv::Vec3b(9, 9, 9));
}

TEST_CASE("Camera interop interfaces are registered for native plugins")
{
    // Load the native plugin and discover its YARP interfaces through interop.
    dinrail::Parameters config;
    config.put("device", "dr_camera_fake");
    dinrail::Device device;
    REQUIRE(device.open(config));

    // All camera adapter registrations must be available on the loaded device.
    yarp::dev::IRGBDSensor* sensor = nullptr;
    yarp::dev::IFrameGrabberControls* controls = nullptr;
    yarp::dev::IFrameGrabberImageRaw* raw = nullptr;
    yarp::dev::IFrameGrabberImage* rgb = nullptr;
    REQUIRE(device.view(sensor));
    REQUIRE(device.view(controls));
    REQUIRE(device.view(raw));
    REQUIRE(device.view(rgb));

    // Acquire a frame through the discovered RGBD interface.
    yarp::sig::FlexImage image;
    REQUIRE(sensor->getRgbImage(image));
    REQUIRE(image.getPixelCode() == VOCAB_PIXEL_RGB);
    REQUIRE(device.close());
}

// Return strided views so the adapters must handle ROI row strides and detach
// their outputs from buffers retained by the source device.
class StridedNativeCamera
    : public dinrail::InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>
{
public:
    using InterfaceAdapter::InterfaceAdapter;
    cv::Mat rgbBacking{4, 5, CV_8UC3, cv::Scalar(7, 33, 99)};
    cv::Mat depthBacking{4, 5, CV_32FC1, cv::Scalar(2.25)};

    dinrail::Status getRgbImage(cv::Mat& image, dinrail::Stamp*) override
    {
        image = rgbBacking(cv::Rect(1, 1, 3, 2));
        return dinrail::OkStatus();
    }

    dinrail::Status getDepthImage(cv::Mat& image, dinrail::Stamp*) override
    {
        image = depthBacking(cv::Rect(1, 1, 3, 2));
        return dinrail::OkStatus();
    }

    dinrail::Status getImages(cv::Mat& rgb,
                              cv::Mat& depth,
                              dinrail::Stamp* rgbStamp,
                              dinrail::Stamp* depthStamp) override
    {
        auto status = getRgbImage(rgb, rgbStamp);
        return status.ok() ? getDepthImage(depth, depthStamp) : status;
    }
};

class StridedNativeFramegrabber : public dinrail::InterfaceAdapter<dinrail::IFrameGrabberImageRaw,
                                                                   yarp::dev::IFrameGrabberImageRaw>
{
public:
    using InterfaceAdapter::InterfaceAdapter;
    cv::Mat backing{4, 5, CV_8UC1, cv::Scalar(42)};

    dinrail::Status getImage(cv::Mat& image) override
    {
        image = backing(cv::Rect(1, 1, 3, 2));
        return dinrail::OkStatus();
    }
};

TEST_CASE("Serial camera conversions handle strided inputs and return owned images")
{
    // Supply non-contiguous RGB and depth frames, with a distinct second row.
    dinrail::DrCameraFake fake;
    REQUIRE(fake.open({}));
    dinrail::InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor> source(fake);
    StridedNativeCamera native(source);
    native.rgbBacking.at<cv::Vec3b>(2, 3) = {11, 22, 55};
    native.depthBacking.at<float>(2, 3) = 4.5f;
    dinrail::InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor> adapter(native);

    // Conversions must preserve pixel positions and the source's RGB channel order.
    yarp::sig::FlexImage rgb;
    yarp::sig::ImageOf<yarp::sig::PixelFloat> depth;
    REQUIRE(adapter.getRgbImage(rgb));
    REQUIRE(adapter.getDepthImage(depth));
    REQUIRE(native.rgbBacking.at<cv::Vec3b>(1, 1) == cv::Vec3b(7, 33, 99));
    REQUIRE(depth.pixel(2, 1) == 4.5f);

    // Row padding is transmitted by YARP and must be initialized explicitly.
    for (size_t row = 0; row < rgb.height(); ++row)
    {
        for (size_t byte = rgb.width() * rgb.getPixelSize(); byte < rgb.getRowSize(); ++byte)
        {
            REQUIRE(rgb.getRawImage()[row * rgb.getRowSize() + byte] == 0);
        }
    }

    // Caller-provided output ROIs retain their row stride and surrounding pixels.
    dinrail::InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor> reverse(adapter);
    cv::Mat rgbOutput(4, 5, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::Mat depthOutput(4, 5, CV_32FC1, cv::Scalar(-1));
    cv::Mat rgbRoi = rgbOutput(cv::Rect(1, 1, 3, 2));
    cv::Mat depthRoi = depthOutput(cv::Rect(1, 1, 3, 2));
    REQUIRE(reverse.getImages(rgbRoi, depthRoi).ok());
    REQUIRE(rgbOutput.at<cv::Vec3b>(2, 3) == cv::Vec3b(11, 22, 55));
    REQUIRE(rgbOutput.at<cv::Vec3b>(0, 0) == cv::Vec3b(255, 255, 255));
    REQUIRE(depthOutput.at<float>(2, 3) == 4.5f);
    REQUIRE(depthOutput.at<float>(0, 0) == -1);

    // The YARP outputs remain valid after the source buffers change.
    native.rgbBacking.setTo(cv::Scalar(0, 0, 0));
    native.depthBacking.setTo(0);
    yarp::sig::ImageOf<yarp::sig::PixelRgb> typedRgb;
    REQUIRE(typedRgb.copy(rgb));
    REQUIRE(typedRgb.pixel(0, 0).r == 7);
    REQUIRE(typedRgb.pixel(2, 1).b == 55);
    REQUIRE(depth.pixel(0, 0) == 2.25f);

    // Monochrome conversion must also honor an arbitrary input row stride.
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw>
        rawSource(fake);
    StridedNativeFramegrabber nativeRaw(rawSource);
    nativeRaw.backing.at<unsigned char>(2, 3) = 73;
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw>
        rawAdapter(nativeRaw);
    yarp::sig::ImageOf<yarp::sig::PixelMono> mono;
    REQUIRE(rawAdapter.getImage(mono));
    dinrail::InterfaceAdapter<dinrail::IFrameGrabberImageRaw, yarp::dev::IFrameGrabberImageRaw>
        reverseRaw(rawAdapter);
    cv::Mat rawOutput(4, 5, CV_8UC1, cv::Scalar(201));
    cv::Mat rawRoi = rawOutput(cv::Rect(1, 1, 3, 2));
    REQUIRE(reverseRaw.getImage(rawRoi).ok());
    REQUIRE(rawOutput.at<unsigned char>(2, 3) == 73);
    REQUIRE(rawOutput.at<unsigned char>(0, 0) == 201);

    nativeRaw.backing.setTo(0);
    REQUIRE(mono.pixel(0, 0) == 42);
    REQUIRE(mono.pixel(2, 1) == 73);
}

// Count pixel-buffer allocations without changing how OpenCV owns/frees them.
// YARP storage is checked separately by verifying its address after repeated reads.
class MatAllocationCounter : public cv::MatAllocator
{
public:
    MatAllocationCounter()
        : m_previous(cv::Mat::getDefaultAllocator())
    {
        cv::Mat::setDefaultAllocator(this);
    }
    ~MatAllocationCounter() override
    {
        cv::Mat::setDefaultAllocator(m_previous);
    }

    cv::UMatData* allocate(int dims,
                           const int* sizes,
                           int type,
                           void* data,
                           size_t* step,
                           cv::AccessFlag flags,
                           cv::UMatUsageFlags usage) const override
    {
        ++allocations;
        return m_previous->allocate(dims, sizes, type, data, step, flags, usage);
    }
    bool allocate(cv::UMatData* data, cv::AccessFlag flags, cv::UMatUsageFlags usage) const override
    {
        ++allocations;
        return m_previous->allocate(data, flags, usage);
    }
    void deallocate(cv::UMatData* data) const override
    {
        m_previous->deallocate(data);
    }

    mutable size_t allocations{0};

private:
    cv::MatAllocator* m_previous;
};

TEST_CASE("Image adapters preserve the process OpenCV thread configuration")
{
    // Set a non-serial policy, then check default and source constructors.
    // Restore the original process configuration even if an assertion fails.
    struct RestoreThreads
    {
        int previous{cv::getNumThreads()};
        ~RestoreThreads()
        {
            cv::setNumThreads(previous);
        }
    } restore;
    cv::setNumThreads(4);
    const int expected = cv::getNumThreads();
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw> raw;
    REQUIRE(cv::getNumThreads() == expected);

    dinrail::InterfaceAdapter<dinrail::IFrameGrabberImageRaw, yarp::dev::IFrameGrabberImageRaw>
        nativeRaw(raw);
    REQUIRE(cv::getNumThreads() == expected);

    dinrail::InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor> rgbd;
    REQUIRE(cv::getNumThreads() == expected);

    dinrail::InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor> nativeRgbd(rgbd);
    REQUIRE(cv::getNumThreads() == expected);

    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImage, dinrail::IRGBDSensor> color;
    REQUIRE(cv::getNumThreads() == expected);
}

TEST_CASE("Image adapters reuse storage after warmup and handle resizing")
{
    // ROI sources avoid allocating themselves, so the counter measures adapters.
    const int expectedThreads = cv::getNumThreads();
    dinrail::DrCameraFake fake;
    REQUIRE(fake.open({}));
    dinrail::InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor> source(fake);
    StridedNativeCamera native(source);
    dinrail::InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor> rgbd(native);
    dinrail::InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor> nativeRgbd(rgbd);
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImage, dinrail::IRGBDSensor> color(native);

    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw>
        rawSource(fake);
    StridedNativeFramegrabber nativeRawSource(rawSource);
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw> raw(
        nativeRawSource);
    dinrail::InterfaceAdapter<dinrail::IFrameGrabberImageRaw, yarp::dev::IFrameGrabberImageRaw>
        nativeRaw(raw);

    yarp::sig::FlexImage rgb;
    yarp::sig::ImageOf<yarp::sig::PixelFloat> depth;
    yarp::sig::ImageOf<yarp::sig::PixelRgb> colorImage;
    yarp::sig::ImageOf<yarp::sig::PixelMono> rawImage;
    cv::Mat rgbMat, depthMat, rawMat;
    auto acquire = [&] {
        bool ok = rgbd.getRgbImage(rgb);
        ok &= rgbd.getDepthImage(depth);
        ok &= rgbd.getImages(rgb, depth);
        ok &= nativeRgbd.getRgbImage(rgbMat).ok();
        ok &= nativeRgbd.getDepthImage(depthMat).ok();
        ok &= nativeRgbd.getImages(rgbMat, depthMat).ok();
        ok &= color.getImage(colorImage);
        ok &= raw.getImage(rawImage);
        ok &= nativeRaw.getImage(rawMat).ok();
        return ok;
    };

    // Allocate once, then retain output pointers to detect storage replacement.
    REQUIRE(acquire());
    const auto* rgbData = rgb.getRawImage();
    const auto* depthData = depth.getRawImage();
    const auto* colorData = colorImage.getRawImage();
    const auto* rawData = rawImage.getRawImage();
    const auto* rgbMatData = rgbMat.data;
    const auto* depthMatData = depthMat.data;
    const auto* rawMatData = rawMat.data;
    bool ok = true;
    size_t allocations;
    {
        MatAllocationCounter counter;
        // Keep Catch2 assertions outside the measured loop.
        for (int frame = 0; frame < 100; ++frame)
        {
            ok &= acquire();
        }
        allocations = counter.allocations;
    }
    REQUIRE(ok);
    REQUIRE(allocations == 0);
    REQUIRE(cv::getNumThreads() == expectedThreads);
    REQUIRE(rgb.getRawImage() == rgbData);
    REQUIRE(depth.getRawImage() == depthData);
    REQUIRE(colorImage.getRawImage() == colorData);
    REQUIRE(rawImage.getRawImage() == rawData);
    REQUIRE(rgbMat.data == rgbMatData);
    REQUIRE(depthMat.data == depthMatData);
    REQUIRE(rawMat.data == rawMatData);

    // Resize in both directions and verify the new geometry and pixels.
    REQUIRE(fake.setRgbResolution(7, 3).ok());
    REQUIRE(fake.setDepthResolution(7, 3).ok());
    dinrail::InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor> resized(source);
    REQUIRE(resized.getImages(rgbMat, depthMat).ok());
    REQUIRE(rgbMat.size() == cv::Size(7, 3));
    REQUIRE(depthMat.size() == cv::Size(7, 3));
    REQUIRE(fake.setRgbResolution(3, 2).ok());
    REQUIRE(fake.setDepthResolution(3, 2).ok());
    REQUIRE(fake.setImages(cv::Mat(2, 3, CV_8UC3, cv::Scalar(11, 22, 55)),
                           cv::Mat(2, 3, CV_32FC1, cv::Scalar(4.5)),
                           {},
                           {})
                .ok());
    REQUIRE(resized.getImages(rgbMat, depthMat).ok());
    REQUIRE(rgbMat.size() == cv::Size(3, 2));
    REQUIRE(rgbMat.at<cv::Vec3b>(1, 2) == cv::Vec3b(11, 22, 55));
    REQUIRE(depthMat.at<float>(1, 2) == 4.5f);
}
