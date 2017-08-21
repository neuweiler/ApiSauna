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
    uint16_t executionTime;
    uint32_t startTime;

};

class Controller
{
public:
    Controller();
    virtual ~Controller();
    void init();
    void loop();
    const SimpleList<TemperatureSensor>& getHiveTempSensors() const;
    const SimpleList<Plate>& getPlates() const;
    const  Humidifier getHumidifier() const;
    const ControllerProgram getProgram() const;
    uint16_t getTimeRunning();
    uint16_t getTimeRemaining();
    void startProgram(ControllerProgram controllerProgram);
    void stopProgram();

private:
    SimpleList<SensorAddress> findTemperatureSensors();
    void assignTemperatureSensors(SimpleList<SensorAddress> addressList);

    SimpleList<Plate> plates;
    SimpleList<PlateConfig> plateConfigs;
    SimpleList<TemperatureSensor> hiveTempSensors;
    Humidifier humidifier;
    ControllerProgram program;
};

#endif /* CONTROLLER_H_ */
