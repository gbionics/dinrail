// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/MultipleAnalogSensorsInterfaces.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>
#include <dinrail/bindings/core/MultipleAnalogSensorsInterfaces.h>

#include <cstddef>
#include <tuple>

namespace dinrail::bindings::core
{

void CreateMultipleAnalogSensorsInterfaces(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using namespace ::dinrail;

    py::enum_<MAS_status>(module, "MAS_status")
        .value("MAS_OK", MAS_OK)
        .value("MAS_ERROR", MAS_ERROR)
        .value("MAS_OVF", MAS_OVF)
        .value("MAS_TIMEOUT", MAS_TIMEOUT)
        .value("MAS_WAITING_FOR_FIRST_READ", MAS_WAITING_FOR_FIRST_READ)
        .value("MAS_UNKNOWN", MAS_UNKNOWN);

    // Sensors providing a measure that is a vector with a fixed size.
    const auto three = constantOutputSize(3);
    const auto six = constantOutputSize(6);
    const auto one = constantOutputSize(1);

    py::class_<IThreeAxisGyroscopes> gyroscopes(module, "IThreeAxisGyroscopes");
    gyroscopes
        .def("getNrOfThreeAxisGyroscopes",
             wrapMethod(&IThreeAxisGyroscopes::getNrOfThreeAxisGyroscopes))
        .def("getThreeAxisGyroscopeStatus",
             wrapMethod(&IThreeAxisGyroscopes::getThreeAxisGyroscopeStatus))
        .def("getThreeAxisGyroscopeName",
             wrapMethod(&IThreeAxisGyroscopes::getThreeAxisGyroscopeName))
        .def("getThreeAxisGyroscopeFrameName",
             wrapMethod(&IThreeAxisGyroscopes::getThreeAxisGyroscopeFrameName))
        .def("getThreeAxisGyroscopeMeasure",
             wrapMethod(&IThreeAxisGyroscopes::getThreeAxisGyroscopeMeasure, three));
    registerInterface(module, gyroscopes);

    py::class_<IThreeAxisGyroscopesSimulation> gyroscopesSimulation(module,
                                                                    "IThreeAxisGyroscopesSimulatio"
                                                                    "n");
    gyroscopesSimulation
        .def("setThreeAxisGyroscopeStatus",
             wrapMethod(&IThreeAxisGyroscopesSimulation::setThreeAxisGyroscopeStatus))
        .def("setThreeAxisGyroscopeMeasure",
             wrapMethod(&IThreeAxisGyroscopesSimulation::setThreeAxisGyroscopeMeasure));
    registerInterface(module, gyroscopesSimulation);

    py::class_<IThreeAxisLinearAccelerometers> linearAccelerometers(module,
                                                                    "IThreeAxisLinearAccelerometer"
                                                                    "s");
    linearAccelerometers
        .def("getNrOfThreeAxisLinearAccelerometers",
             wrapMethod(&IThreeAxisLinearAccelerometers::getNrOfThreeAxisLinearAccelerometers))
        .def("getThreeAxisLinearAccelerometerStatus",
             wrapMethod(&IThreeAxisLinearAccelerometers::getThreeAxisLinearAccelerometerStatus))
        .def("getThreeAxisLinearAccelerometerName",
             wrapMethod(&IThreeAxisLinearAccelerometers::getThreeAxisLinearAccelerometerName))
        .def("getThreeAxisLinearAccelerometerFrameName",
             wrapMethod(&IThreeAxisLinearAccelerometers::getThreeAxisLinearAccelerometerFrameName))
        .def("getThreeAxisLinearAccelerometerMeasure",
             wrapMethod(&IThreeAxisLinearAccelerometers::getThreeAxisLinearAccelerometerMeasure,
                        three));
    registerInterface(module, linearAccelerometers);

    py::class_<IThreeAxisLinearAccelerometersSimulation> linearAccelerometersSimulation(module,
                                                                                        "IThreeAxis"
                                                                                        "LinearAcce"
                                                                                        "lerometers"
                                                                                        "Simulatio"
                                                                                        "n");
    linearAccelerometersSimulation
        .def("setThreeAxisLinearAccelerometerStatus",
             wrapMethod(
                 &IThreeAxisLinearAccelerometersSimulation::setThreeAxisLinearAccelerometerStatus))
        .def("setThreeAxisLinearAccelerometerMeasure",
             wrapMethod(
                 &IThreeAxisLinearAccelerometersSimulation::setThreeAxisLinearAccelerometerMeasure));
    registerInterface(module, linearAccelerometersSimulation);

    py::class_<IThreeAxisAngularAccelerometers> angularAccelerometers(module,
                                                                      "IThreeAxisAngularAcceleromet"
                                                                      "ers");
    angularAccelerometers
        .def("getNrOfThreeAxisAngularAccelerometers",
             wrapMethod(&IThreeAxisAngularAccelerometers::getNrOfThreeAxisAngularAccelerometers))
        .def("getThreeAxisAngularAccelerometerStatus",
             wrapMethod(&IThreeAxisAngularAccelerometers::getThreeAxisAngularAccelerometerStatus))
        .def("getThreeAxisAngularAccelerometerName",
             wrapMethod(&IThreeAxisAngularAccelerometers::getThreeAxisAngularAccelerometerName))
        .def("getThreeAxisAngularAccelerometerFrameName",
             wrapMethod(
                 &IThreeAxisAngularAccelerometers::getThreeAxisAngularAccelerometerFrameName))
        .def("getThreeAxisAngularAccelerometerMeasure",
             wrapMethod(&IThreeAxisAngularAccelerometers::getThreeAxisAngularAccelerometerMeasure,
                        three));
    registerInterface(module, angularAccelerometers);

    py::class_<IThreeAxisAngularAccelerometersSimulation> angularAccelerometersSimulation(module,
                                                                                          "IThreeAx"
                                                                                          "isAngula"
                                                                                          "rAcceler"
                                                                                          "ometersS"
                                                                                          "imulatio"
                                                                                          "n");
    angularAccelerometersSimulation
        .def("setThreeAxisAngularAccelerometerStatus",
             wrapMethod(
                 &IThreeAxisAngularAccelerometersSimulation::setThreeAxisAngularAccelerometerStatus))
        .def("setThreeAxisAngularAccelerometerMeasure",
             wrapMethod(&IThreeAxisAngularAccelerometersSimulation::
                            setThreeAxisAngularAccelerometerMeasure));
    registerInterface(module, angularAccelerometersSimulation);

    py::class_<IThreeAxisMagnetometers> magnetometers(module, "IThreeAxisMagnetometers");
    magnetometers
        .def("getNrOfThreeAxisMagnetometers",
             wrapMethod(&IThreeAxisMagnetometers::getNrOfThreeAxisMagnetometers))
        .def("getThreeAxisMagnetometerStatus",
             wrapMethod(&IThreeAxisMagnetometers::getThreeAxisMagnetometerStatus))
        .def("getThreeAxisMagnetometerName",
             wrapMethod(&IThreeAxisMagnetometers::getThreeAxisMagnetometerName))
        .def("getThreeAxisMagnetometerFrameName",
             wrapMethod(&IThreeAxisMagnetometers::getThreeAxisMagnetometerFrameName))
        .def("getThreeAxisMagnetometerMeasure",
             wrapMethod(&IThreeAxisMagnetometers::getThreeAxisMagnetometerMeasure, three));
    registerInterface(module, magnetometers);

    py::class_<IThreeAxisMagnetometersSimulation> magnetometersSimulation(module,
                                                                          "IThreeAxisMagnetometersS"
                                                                          "imulation");
    magnetometersSimulation
        .def("setThreeAxisMagnetometerStatus",
             wrapMethod(&IThreeAxisMagnetometersSimulation::setThreeAxisMagnetometerStatus))
        .def("setThreeAxisMagnetometerMeasure",
             wrapMethod(&IThreeAxisMagnetometersSimulation::setThreeAxisMagnetometerMeasure));
    registerInterface(module, magnetometersSimulation);

    py::class_<IPositionSensors> positionSensors(module, "IPositionSensors");
    positionSensors
        .def("getNrOfPositionSensors", wrapMethod(&IPositionSensors::getNrOfPositionSensors))
        .def("getPositionSensorStatus", wrapMethod(&IPositionSensors::getPositionSensorStatus))
        .def("getPositionSensorName", wrapMethod(&IPositionSensors::getPositionSensorName))
        .def("getPositionSensorFrameName",
             wrapMethod(&IPositionSensors::getPositionSensorFrameName))
        .def("getPositionSensorMeasure",
             wrapMethod(&IPositionSensors::getPositionSensorMeasure, three));
    registerInterface(module, positionSensors);

    py::class_<IPositionSensorsSimulation> positionSensorsSimulation(module,
                                                                     "IPositionSensorsSimulation");
    positionSensorsSimulation
        .def("setPositionSensorStatus",
             wrapMethod(&IPositionSensorsSimulation::setPositionSensorStatus))
        .def("setPositionSensorMeasure",
             wrapMethod(&IPositionSensorsSimulation::setPositionSensorMeasure));
    registerInterface(module, positionSensorsSimulation);

    py::class_<ILinearVelocitySensors> linearVelocitySensors(module, "ILinearVelocitySensors");
    linearVelocitySensors
        .def("getNrOfLinearVelocitySensors",
             wrapMethod(&ILinearVelocitySensors::getNrOfLinearVelocitySensors))
        .def("getLinearVelocitySensorStatus",
             wrapMethod(&ILinearVelocitySensors::getLinearVelocitySensorStatus))
        .def("getLinearVelocitySensorName",
             wrapMethod(&ILinearVelocitySensors::getLinearVelocitySensorName))
        .def("getLinearVelocitySensorFrameName",
             wrapMethod(&ILinearVelocitySensors::getLinearVelocitySensorFrameName))
        .def("getLinearVelocitySensorMeasure",
             wrapMethod(&ILinearVelocitySensors::getLinearVelocitySensorMeasure, three));
    registerInterface(module, linearVelocitySensors);

    py::class_<ILinearVelocitySensorsSimulation> linearVelocitySensorsSimulation(module,
                                                                                 "ILinearVelocitySe"
                                                                                 "nsorsSimulation");
    linearVelocitySensorsSimulation
        .def("setLinearVelocitySensorStatus",
             wrapMethod(&ILinearVelocitySensorsSimulation::setLinearVelocitySensorStatus))
        .def("setLinearVelocitySensorMeasure",
             wrapMethod(&ILinearVelocitySensorsSimulation::setLinearVelocitySensorMeasure));
    registerInterface(module, linearVelocitySensorsSimulation);

    py::class_<IOrientationSensors> orientationSensors(module, "IOrientationSensors");
    orientationSensors
        .def("getNrOfOrientationSensors",
             wrapMethod(&IOrientationSensors::getNrOfOrientationSensors))
        .def("getOrientationSensorStatus",
             wrapMethod(&IOrientationSensors::getOrientationSensorStatus))
        .def("getOrientationSensorName", wrapMethod(&IOrientationSensors::getOrientationSensorName))
        .def("getOrientationSensorFrameName",
             wrapMethod(&IOrientationSensors::getOrientationSensorFrameName))
        .def("getOrientationSensorMeasureAsRollPitchYaw",
             wrapMethod(&IOrientationSensors::getOrientationSensorMeasureAsRollPitchYaw, three));
    registerInterface(module, orientationSensors);

    py::class_<IOrientationSensorsSimulation> orientationSensorsSimulation(module,
                                                                           "IOrientationSensorsSimu"
                                                                           "lation");
    orientationSensorsSimulation
        .def("setOrientationSensorStatus",
             wrapMethod(&IOrientationSensorsSimulation::setOrientationSensorStatus))
        .def("setOrientationSensorMeasureAsRollPitchYaw",
             wrapMethod(&IOrientationSensorsSimulation::setOrientationSensorMeasureAsRollPitchYaw));
    registerInterface(module, orientationSensorsSimulation);

    py::class_<ITemperatureSensors> temperatureSensors(module, "ITemperatureSensors");
    temperatureSensors
        .def("getNrOfTemperatureSensors",
             wrapMethod(&ITemperatureSensors::getNrOfTemperatureSensors))
        .def("getTemperatureSensorStatus",
             wrapMethod(&ITemperatureSensors::getTemperatureSensorStatus))
        .def("getTemperatureSensorName", wrapMethod(&ITemperatureSensors::getTemperatureSensorName))
        .def("getTemperatureSensorFrameName",
             wrapMethod(&ITemperatureSensors::getTemperatureSensorFrameName))
        .def("getTemperatureSensorMeasure",
             wrapMethod(
                 static_cast<bool (ITemperatureSensors::*)(std::size_t, double&, double&) const>(
                     &ITemperatureSensors::getTemperatureSensorMeasure)))
        .def("getTemperatureSensorMeasureAsVector",
             wrapMethod(static_cast<bool (
                            ITemperatureSensors::*)(std::size_t, VectorProxy<double>::Ref, double&)
                                        const>(&ITemperatureSensors::getTemperatureSensorMeasure),
                        one));
    registerInterface(module, temperatureSensors);

    py::class_<ITemperatureSensorsSimulation> temperatureSensorsSimulation(module,
                                                                           "ITemperatureSensorsSimu"
                                                                           "lation");
    temperatureSensorsSimulation
        .def("setTemperatureSensorStatus",
             wrapMethod(&ITemperatureSensorsSimulation::setTemperatureSensorStatus))
        .def("setTemperatureSensorMeasure",
             wrapMethod(&ITemperatureSensorsSimulation::setTemperatureSensorMeasure));
    registerInterface(module, temperatureSensorsSimulation);

    py::class_<ISixAxisForceTorqueSensors> forceTorqueSensors(module, "ISixAxisForceTorqueSensors");
    forceTorqueSensors
        .def("getNrOfSixAxisForceTorqueSensors",
             wrapMethod(&ISixAxisForceTorqueSensors::getNrOfSixAxisForceTorqueSensors))
        .def("getSixAxisForceTorqueSensorStatus",
             wrapMethod(&ISixAxisForceTorqueSensors::getSixAxisForceTorqueSensorStatus))
        .def("getSixAxisForceTorqueSensorName",
             wrapMethod(&ISixAxisForceTorqueSensors::getSixAxisForceTorqueSensorName))
        .def("getSixAxisForceTorqueSensorFrameName",
             wrapMethod(&ISixAxisForceTorqueSensors::getSixAxisForceTorqueSensorFrameName))
        .def("getSixAxisForceTorqueSensorMeasure",
             wrapMethod(&ISixAxisForceTorqueSensors::getSixAxisForceTorqueSensorMeasure, six));
    registerInterface(module, forceTorqueSensors);

    py::class_<ISixAxisForceTorqueSensorsSimulation> forceTorqueSensorsSimulation(module,
                                                                                  "ISixAxisForceTor"
                                                                                  "queSensorsSimula"
                                                                                  "tion");
    forceTorqueSensorsSimulation
        .def("setSixAxisForceTorqueSensorStatus",
             wrapMethod(&ISixAxisForceTorqueSensorsSimulation::setSixAxisForceTorqueSensorStatus))
        .def("setSixAxisForceTorqueSensorMeasure",
             wrapMethod(&ISixAxisForceTorqueSensorsSimulation::setSixAxisForceTorqueSensorMeasure));
    registerInterface(module, forceTorqueSensorsSimulation);

    // Sensors providing a measure that is a vector whose size depends on the sensor.
    py::class_<IContactLoadCellArrays> contactLoadCellArrays(module, "IContactLoadCellArrays");
    contactLoadCellArrays
        .def("getNrOfContactLoadCellArrays",
             wrapMethod(&IContactLoadCellArrays::getNrOfContactLoadCellArrays))
        .def("getContactLoadCellArrayStatus",
             wrapMethod(&IContactLoadCellArrays::getContactLoadCellArrayStatus))
        .def("getContactLoadCellArrayName",
             wrapMethod(&IContactLoadCellArrays::getContactLoadCellArrayName))
        .def("getContactLoadCellArrayMeasure",
             wrapMethod(&IContactLoadCellArrays::getContactLoadCellArrayMeasure,
                        [](IContactLoadCellArrays& self, const auto& inputs) -> std::size_t {
                            return self.getContactLoadCellArraySize(std::get<0>(inputs));
                        }))
        .def("getContactLoadCellArraySize",
             wrapMethod(&IContactLoadCellArrays::getContactLoadCellArraySize));
    registerInterface(module, contactLoadCellArrays);

    py::class_<IContactLoadCellArraysSimulation> contactLoadCellArraysSimulation(module,
                                                                                 "IContactLoadCellA"
                                                                                 "rraysSimulation");
    contactLoadCellArraysSimulation
        .def("setContactLoadCellArrayStatus",
             wrapMethod(&IContactLoadCellArraysSimulation::setContactLoadCellArrayStatus))
        .def("setContactLoadCellArrayMeasure",
             wrapMethod(&IContactLoadCellArraysSimulation::setContactLoadCellArrayMeasure));
    registerInterface(module, contactLoadCellArraysSimulation);

    py::class_<IEncoderArrays> encoderArrays(module, "IEncoderArrays");
    encoderArrays.def("getNrOfEncoderArrays", wrapMethod(&IEncoderArrays::getNrOfEncoderArrays))
        .def("getEncoderArrayStatus", wrapMethod(&IEncoderArrays::getEncoderArrayStatus))
        .def("getEncoderArrayName", wrapMethod(&IEncoderArrays::getEncoderArrayName))
        .def("getEncoderArrayMeasure",
             wrapMethod(&IEncoderArrays::getEncoderArrayMeasure,
                        [](IEncoderArrays& self, const auto& inputs) -> std::size_t {
                            return self.getEncoderArraySize(std::get<0>(inputs));
                        }))
        .def("getEncoderArraySize", wrapMethod(&IEncoderArrays::getEncoderArraySize));
    registerInterface(module, encoderArrays);

    py::class_<IEncoderArraysSimulation> encoderArraysSimulation(module,
                                                                 "IEncoderArraysSimulation");
    encoderArraysSimulation
        .def("setEncoderArrayStatus", wrapMethod(&IEncoderArraysSimulation::setEncoderArrayStatus))
        .def("setEncoderArrayMeasure",
             wrapMethod(&IEncoderArraysSimulation::setEncoderArrayMeasure));
    registerInterface(module, encoderArraysSimulation);

    py::class_<ISkinPatches> skinPatches(module, "ISkinPatches");
    skinPatches.def("getNrOfSkinPatches", wrapMethod(&ISkinPatches::getNrOfSkinPatches))
        .def("getSkinPatchStatus", wrapMethod(&ISkinPatches::getSkinPatchStatus))
        .def("getSkinPatchName", wrapMethod(&ISkinPatches::getSkinPatchName))
        .def("getSkinPatchMeasure",
             wrapMethod(&ISkinPatches::getSkinPatchMeasure,
                        [](ISkinPatches& self, const auto& inputs) -> std::size_t {
                            return self.getSkinPatchSize(std::get<0>(inputs));
                        }))
        .def("getSkinPatchSize", wrapMethod(&ISkinPatches::getSkinPatchSize));
    registerInterface(module, skinPatches);

    py::class_<ISkinPatchesSimulation> skinPatchesSimulation(module, "ISkinPatchesSimulation");
    skinPatchesSimulation
        .def("setSkinPatchStatus", wrapMethod(&ISkinPatchesSimulation::setSkinPatchStatus))
        .def("setSkinPatchMeasure", wrapMethod(&ISkinPatchesSimulation::setSkinPatchMeasure));
    registerInterface(module, skinPatchesSimulation);
}

} // namespace dinrail::bindings::core
