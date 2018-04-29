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
#include "Configuration.h"
#include "TemperatureSensor.h"
#include "Humidifier.h"
#include "Plate.h"
#include "Status.h"
#include "Beeper.h"
#include "PID_v1.h"
#include "SerialConsole.h"
#include "HID.h"
#include "ProgramHandler.h"

class Controller: public ProgramObserver
{
public:
    static Controller *getSetupLoopInstance();
    virtual ~Controller();
    void initialize();
    void process();
    void handleEvent(ProgramEvent event, Program *program);
    int16_t getHiveTargetTemperature();
    void setPIDTuningHive(double kp, double ki, double kd);
    void setPIDTuningPlate(double kp, double ki, double kd);
    void setFanSpeed(uint8_t speed);
    void setFanSpeedHumidifier(uint8_t speed);
    void setHumidifierLimits(uint8_t min, uint8_t max);

private:
    Controller();
    Controller(Controller const&); // copy disabled
    void operator=(Controller const&); // assigment disabled
    void initOutput();
    void initPrograms();
    void powerDownDevices();
    SimpleList<SensorAddress> detectTemperatureSensors();
    bool containsSensorAddress(SimpleList<SensorAddress> &addressList, SensorAddress address);
    bool assignPlateSensors(SimpleList<SensorAddress> &addressList);
    bool assignHiveSensors(SimpleList<SensorAddress> &addressList);
    int16_t retrieveHiveTemperatures();
    int16_t calculatePlateTargetTemperature();
    void updateProgramState();
    void initPid();

    SimpleList<Plate> plates;
    SimpleList<TemperatureSensor> hiveTempSensors;
    Humidifier humidifier;
    Beeper beeper;
    HID hid;
    SerialConsole serialConsole;
    double actualTemperature, targetTemperature, plateTemperature; // values for/set by the PID controller
    PID *pid; // pointer to PID controller
    bool statusLed;
    uint8_t tickCounter;
};

#endif /* CONTROLLER_H_ */
