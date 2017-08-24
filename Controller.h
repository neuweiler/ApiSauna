/*
 * Controller.h
 *
 Copyright (c) 2017 Michael Neuweiler

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "SimpleList.h"
#include "TemperatureSensor.h"
#include "Humidifier.h"
#include "Plate.h"
#include "Status.h"

class ControllerProgram {
public:
    int16_t preHeatTemperature; // the target hive temperature during pre-heat (in 0.1 deg C)
    uint8_t preHeatFanSpeed; // the maximum fan speed during pre-heat (0-255)
    int16_t hiveTemperature; // the target temperature (in 0.1 deg C)
    int16_t deratingHiveDelta; // delta of the hive temperature where the derating begins (in 0.1 deg C)
    int16_t plateTemperature; // the maximum temperature of the heater plates (in 0.1 deg C)
    int16_t deratingPlateDelta; // delta of the plate temperature where the derating begins (in 0.1 deg C)
    uint16_t preHeatDuration; // the duration of the pre-heating cycle (in sec)
    uint16_t duration; // how long the program should run (in sec)
    uint8_t fanSpeed; // the maximum fan speed (0-255)
    uint8_t fanSpeedHumidifier; // the fan speed of the humidifier fan (when active (0-255)
    uint8_t humidityMinimum; // the minimum humidity in %
    uint8_t humidityMaximum; // the maximum humidity in %

    uint32_t startTime; // timestamp when the program started (in millis)
};

class Controller
{
public:
    Controller();
    virtual ~Controller();
    void init();
    void loop();
    SimpleList<TemperatureSensor> *getHiveTempSensors();
    SimpleList<Plate> *getPlates();
    Humidifier *getHumidifier();
    ControllerProgram *getProgram();
    int16_t getHiveTemperature();
    int16_t getHiveTargetTemperature();
    uint16_t getTimeRunning();
    uint16_t getTimeRemaining();
    void startProgram(ControllerProgram controllerProgram);
    void stopProgram();

private:
    SimpleList<SensorAddress> findTemperatureSensors();
    void assignTemperatureSensors(SimpleList<SensorAddress> *addressList);
    void powerDownDevices();
    int16_t retrieveHiveTemperatures();
    int16_t calculateMaxPlateTemperature();
    void updateProgramState();

    SimpleList<Plate> plates;
    SimpleList<PlateConfig> plateConfigs;
    SimpleList<TemperatureSensor> hiveTempSensors;
    Humidifier humidifier;
    ControllerProgram program;
    int16_t hiveTemperature;
    bool statusLed;
};

#endif /* CONTROLLER_H_ */
