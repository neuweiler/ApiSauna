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
#include "Beeper.h"
#include "SerialConsole.h"
#include "PID_v1.h"
//#include "EEPROM.h"

class ControllerProgram {
public:
    char name[16]; // name to be displayed in menu
    int16_t temperaturePreHeat; // the target hive temperature during pre-heat (in 0.1 deg C)
    int16_t temperatureHive; // the target hive temperature (in 0.1 deg C)
    double hiveKp, hiveKi, hiveKd; // hive temperature PID configuration
    int16_t temperaturePlate; // the target temperature of the heater plates (in 0.1 deg C)
    double plateKp, plateKi, plateKd; // plate temperature PID configuration
    uint8_t fanSpeedPreHeat; // the fan speed during pre-heat (0-255)
    uint8_t fanSpeed; // the fan speed (0-255)
    uint16_t durationPreHeat; // the duration of the pre-heating cycle (in min)
    uint16_t duration; // the duration of the program (in min)
    uint8_t fanSpeedHumidifier; // the fan speed of the humidifier fan (when active (0-255)
    uint8_t humidityMinimum; // the minimum relative humidity in %
    uint8_t humidityMaximum; // the maximum relative humidity in %
};

class Controller
{
public:
    Controller();
    virtual ~Controller();
    void init();
    void loadDefaults();
    void loadConfig();
    void saveConfig();
    void loop();
    SimpleList<TemperatureSensor> *getHiveTempSensors();
    SimpleList<Plate> *getPlates();
    Humidifier *getHumidifier();
    SimpleList<ControllerProgram> *getPrograms();
    ControllerProgram *getRunningProgram();
    int16_t getHiveTemperature();
    int16_t getHiveTargetTemperature();
    uint32_t getTimeRunning();
    uint32_t getTimeRemaining();
    void startProgram(uint8_t programNumber);
    void stopProgram();
    void setPIDTuningHive(double kp, double ki, double kd);
    void setPIDTuningPlate(double kp, double ki, double kd);
    void setFanSpeed(uint8_t speed);
    void setFanSpeedHumidifier(uint8_t speed);
    void setHumidifierLimits(uint8_t min, uint8_t max);


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
    Beeper beeper;
    SimpleList<ControllerProgram> programs;
    ControllerProgram *runningProgram;
    double actualTemperature, targetTemperature, plateTemperature;
    PID *pid; // pointer to PID controller
    bool statusLed;
    uint32_t startTime; // timestamp when the program started (in millis)
};

extern Controller controller;

#endif /* CONTROLLER_H_ */
