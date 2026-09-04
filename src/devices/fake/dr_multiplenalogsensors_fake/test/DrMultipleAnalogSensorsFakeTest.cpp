// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/MultipleAnalogSensorsInterfaces.h>
#include <dinrail/Parameters.h>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("dr_multiplenalogsensors_fake exposes all multiple analog sensor interfaces",
          "[device][multiple_analog_sensors]")
{
    dinrail::Parameters options;
    options.put("device", "dr_multiplenalogsensors_fake");
    options.put("number_of_sensors", 2);
    options.put("three_axis_gyroscope_names", std::vector<std::string>{"imu", "imu_1"});
    options.put("three_axis_gyroscope_frame_names",
                std::vector<std::string>{"imu_frame", "imu_1_frame"});

    dinrail::Device device;
    REQUIRE(device.open(options));

    dinrail::IThreeAxisGyroscopes* gyroscopes = nullptr;
    dinrail::IThreeAxisGyroscopesSimulation* gyroscopesSimulation = nullptr;
    REQUIRE(device.view(gyroscopes));
    REQUIRE(device.view(gyroscopesSimulation));
    REQUIRE(gyroscopes->getNrOfThreeAxisGyroscopes() == 2);
    REQUIRE(gyroscopes->getThreeAxisGyroscopeStatus(0) == dinrail::MAS_OK);
    REQUIRE(gyroscopesSimulation->setThreeAxisGyroscopeStatus(0, dinrail::MAS_TIMEOUT));
    const std::vector<double> gyro{1.0, 2.0, 3.0};
    REQUIRE(gyroscopesSimulation->setThreeAxisGyroscopeMeasure(0, gyro, 12.5));
    std::vector<double> gyroOut;
    double timestamp = 0.0;
    REQUIRE(gyroscopes->getThreeAxisGyroscopeMeasure(0, gyroOut, timestamp));
    REQUIRE(gyroOut == gyro);
    REQUIRE(timestamp == 12.5);
    REQUIRE(gyroscopes->getThreeAxisGyroscopeStatus(0) == dinrail::MAS_TIMEOUT);
    std::string name;
    std::string frame;
    REQUIRE(gyroscopes->getThreeAxisGyroscopeName(0, name));
    REQUIRE(gyroscopes->getThreeAxisGyroscopeFrameName(0, frame));
    REQUIRE(name == "imu");
    REQUIRE(frame == "imu_frame");

    dinrail::IThreeAxisLinearAccelerometers* linearAccelerometers = nullptr;
    dinrail::IThreeAxisLinearAccelerometersSimulation* linearAccelerometersSimulation = nullptr;
    REQUIRE(device.view(linearAccelerometers));
    REQUIRE(device.view(linearAccelerometersSimulation));
    REQUIRE(
        linearAccelerometersSimulation
            ->setThreeAxisLinearAccelerometerMeasure(1, std::vector<double>{4.0, 5.0, 6.0}, 13.0));
    std::vector<double> linearAcceleration;
    REQUIRE(linearAccelerometers->getThreeAxisLinearAccelerometerMeasure(1,
                                                                         linearAcceleration,
                                                                         timestamp));
    REQUIRE(linearAcceleration == std::vector<double>{4.0, 5.0, 6.0});
    REQUIRE(timestamp == 13.0);

    dinrail::IThreeAxisAngularAccelerometers* angularAccelerometers = nullptr;
    dinrail::IThreeAxisAngularAccelerometersSimulation* angularAccelerometersSimulation = nullptr;
    REQUIRE(device.view(angularAccelerometers));
    REQUIRE(device.view(angularAccelerometersSimulation));
    REQUIRE(
        angularAccelerometersSimulation
            ->setThreeAxisAngularAccelerometerMeasure(0, std::vector<double>{7.0, 8.0, 9.0}, 14.0));
    std::vector<double> angularAcceleration;
    REQUIRE(angularAccelerometers->getThreeAxisAngularAccelerometerMeasure(0,
                                                                           angularAcceleration,
                                                                           timestamp));
    REQUIRE(angularAcceleration == std::vector<double>{7.0, 8.0, 9.0});

    dinrail::IThreeAxisMagnetometers* magnetometers = nullptr;
    dinrail::IThreeAxisMagnetometersSimulation* magnetometersSimulation = nullptr;
    REQUIRE(device.view(magnetometers));
    REQUIRE(device.view(magnetometersSimulation));
    REQUIRE(magnetometersSimulation
                ->setThreeAxisMagnetometerMeasure(0, std::vector<double>{10.0, 11.0, 12.0}, 15.0));
    std::vector<double> magneticField;
    REQUIRE(magnetometers->getThreeAxisMagnetometerMeasure(0, magneticField, timestamp));
    REQUIRE(magneticField == std::vector<double>{10.0, 11.0, 12.0});

    dinrail::IPositionSensors* positions = nullptr;
    dinrail::IPositionSensorsSimulation* positionsSimulation = nullptr;
    REQUIRE(device.view(positions));
    REQUIRE(device.view(positionsSimulation));
    REQUIRE(
        positionsSimulation->setPositionSensorMeasure(0, std::vector<double>{1.0, 2.0, 3.0}, 16.0));
    std::vector<double> position;
    REQUIRE(positions->getPositionSensorMeasure(0, position, timestamp));
    REQUIRE(position == std::vector<double>{1.0, 2.0, 3.0});

    dinrail::ILinearVelocitySensors* velocities = nullptr;
    dinrail::ILinearVelocitySensorsSimulation* velocitiesSimulation = nullptr;
    REQUIRE(device.view(velocities));
    REQUIRE(device.view(velocitiesSimulation));
    REQUIRE(velocitiesSimulation->setLinearVelocitySensorMeasure(0,
                                                                 std::vector<double>{4.0, 5.0, 6.0},
                                                                 17.0));
    std::vector<double> velocity;
    REQUIRE(velocities->getLinearVelocitySensorMeasure(0, velocity, timestamp));
    REQUIRE(velocity == std::vector<double>{4.0, 5.0, 6.0});

    dinrail::IOrientationSensors* orientations = nullptr;
    dinrail::IOrientationSensorsSimulation* orientationsSimulation = nullptr;
    REQUIRE(device.view(orientations));
    REQUIRE(device.view(orientationsSimulation));
    REQUIRE(orientationsSimulation
                ->setOrientationSensorMeasureAsRollPitchYaw(0,
                                                            std::vector<double>{20.0, 21.0, 22.0},
                                                            18.0));
    std::vector<double> rpy;
    REQUIRE(orientations->getOrientationSensorMeasureAsRollPitchYaw(0, rpy, timestamp));
    REQUIRE(rpy == std::vector<double>{20.0, 21.0, 22.0});

    dinrail::ITemperatureSensors* temperatures = nullptr;
    dinrail::ITemperatureSensorsSimulation* temperaturesSimulation = nullptr;
    REQUIRE(device.view(temperatures));
    REQUIRE(device.view(temperaturesSimulation));
    REQUIRE(temperaturesSimulation->setTemperatureSensorMeasure(0, 25.5, 19.0));
    double temperature = 0.0;
    REQUIRE(temperatures->getTemperatureSensorMeasure(0, temperature, timestamp));
    REQUIRE(temperature == 25.5);
    REQUIRE(timestamp == 19.0);
    std::vector<double> temperatureVector;
    REQUIRE(temperatures->getTemperatureSensorMeasure(0, temperatureVector, timestamp));
    REQUIRE(temperatureVector == std::vector<double>{25.5});

    dinrail::ISixAxisForceTorqueSensors* forceTorques = nullptr;
    dinrail::ISixAxisForceTorqueSensorsSimulation* forceTorquesSimulation = nullptr;
    REQUIRE(device.view(forceTorques));
    REQUIRE(device.view(forceTorquesSimulation));
    REQUIRE(
        forceTorquesSimulation
            ->setSixAxisForceTorqueSensorMeasure(0,
                                                 std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0},
                                                 20.0));
    std::vector<double> forceTorque;
    REQUIRE(forceTorques->getSixAxisForceTorqueSensorMeasure(0, forceTorque, timestamp));
    REQUIRE(forceTorque == std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0});

    dinrail::IContactLoadCellArrays* contact = nullptr;
    dinrail::IContactLoadCellArraysSimulation* contactSimulation = nullptr;
    REQUIRE(device.view(contact));
    REQUIRE(device.view(contactSimulation));
    REQUIRE(contact->getContactLoadCellArraySize(0) == 4);
    REQUIRE(contactSimulation
                ->setContactLoadCellArrayMeasure(0, std::vector<double>{1.0, 2.0, 3.0, 4.0}, 21.0));
    std::vector<double> contactValues;
    REQUIRE(contact->getContactLoadCellArrayMeasure(0, contactValues, timestamp));
    REQUIRE(contactValues == std::vector<double>{1.0, 2.0, 3.0, 4.0});

    dinrail::IEncoderArrays* encoderArrays = nullptr;
    dinrail::IEncoderArraysSimulation* encoderArraysSimulation = nullptr;
    REQUIRE(device.view(encoderArrays));
    REQUIRE(device.view(encoderArraysSimulation));
    REQUIRE(encoderArrays->getEncoderArraySize(0) == 4);
    REQUIRE(encoderArraysSimulation->setEncoderArrayMeasure(0,
                                                            std::vector<double>{5.0, 6.0, 7.0, 8.0},
                                                            22.0));
    std::vector<double> encoderValues;
    REQUIRE(encoderArrays->getEncoderArrayMeasure(0, encoderValues, timestamp));
    REQUIRE(encoderValues == std::vector<double>{5.0, 6.0, 7.0, 8.0});

    dinrail::ISkinPatches* skin = nullptr;
    dinrail::ISkinPatchesSimulation* skinSimulation = nullptr;
    REQUIRE(device.view(skin));
    REQUIRE(device.view(skinSimulation));
    REQUIRE(skin->getSkinPatchSize(0) == 4);
    REQUIRE(
        skinSimulation->setSkinPatchMeasure(0, std::vector<double>{9.0, 10.0, 11.0, 12.0}, 23.0));
    std::vector<double> skinValues;
    REQUIRE(skin->getSkinPatchMeasure(0, skinValues, timestamp));
    REQUIRE(skinValues == std::vector<double>{9.0, 10.0, 11.0, 12.0});

    REQUIRE_FALSE(gyroscopes->getThreeAxisGyroscopeName(2, name));
    REQUIRE_FALSE(gyroscopesSimulation
                      ->setThreeAxisGyroscopeMeasure(2, std::vector<double>{0.0, 0.0, 0.0}, 0.0));
    REQUIRE(device.close());
}

TEST_CASE("multiple analog fake rejects invalid dimensions and counts",
          "[device][multiple_analog_sensors]")
{
    dinrail::Parameters options;
    options.put("device", "dr_multiplenalogsensors_fake");
    options.put("number_of_sensors", -1);

    dinrail::Device device;
    REQUIRE_FALSE(device.open(options));

    options.put("number_of_sensors", 1);
    REQUIRE(device.open(options));
    dinrail::IThreeAxisGyroscopesSimulation* simulation = nullptr;
    REQUIRE(device.view(simulation));
    REQUIRE_FALSE(simulation->setThreeAxisGyroscopeMeasure(0, std::vector<double>{1.0, 2.0}, 0.0));
    REQUIRE(device.close());
}

TEST_CASE("multiple analog fake validates configured metadata", "[device][multiple_analog_sensors]")
{
    dinrail::Parameters options;
    options.put("device", "dr_multiplenalogsensors_fake");
    options.put("number_of_sensors", 2);
    options.put("three_axis_gyroscope_names", std::vector<std::string>{"imu"});

    dinrail::Device device;
    REQUIRE_FALSE(device.open(options));
}
