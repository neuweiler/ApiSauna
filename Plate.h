/*
 * Plate.h
 *
 Copyright (c) 2017-2021 Michael Neuweiler

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

#ifndef PLATE_H_
#define PLATE_H_

#include <Arduino.h>
#include "Fan.h"
#include "Heater.h"
#include "TemperatureSensor.h"
#include "PID_v1.h"

#include "EventHandler.h"
#include "Program.h"

class Plate: EventListener
{
public:
    Plate();
    Plate(uint8_t number);
    virtual ~Plate();
    void initialize();
    void handleEvent(Event event, ...);
    void setTargetTemperature(int16_t temperature);

private:
    void process();
    uint8_t calculateHeaterPower();
    void programChange(const Program& program);

    static uint8_t activeHeaters; // a static counter to establish how many heaters are active in non-PWM mode
    uint8_t number;
    TemperatureSensor sensor;
    Heater heater;
    Fan fan;
    double targetTemperature, currentTemperature, power;
    PID *pid; // pointer to PID controller
    bool paused; // flag indicating if the plate is in paused mode
    bool running; // is a program running?
};

#endif /* PLATE_H_ */
