/*
 * Plate.h
 *
 * Class to which TickObserver objects can register to be triggered
 * on a certain interval.
 * TickObserver with the same interval are grouped to the same timer
 * and triggered in sequence per timer interrupt.
 *
 * NOTE: The initialize() method must be called before a observer is registered !
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

#ifndef PLATE_H_
#define PLATE_H_

#include "Device.h"
#include "Fan.h"
#include "Heater.h"
#include "TemperatureSensor.h"
#include "PID_v1.h"

class Plate: Device
{
public:
    Plate();
    void initialize();
    void initialize(uint8_t index);
    virtual ~Plate();
    void process();
    void setTargetTemperature(int16_t temperature);
    int16_t getTargetTemperature();
    void setMaximumPower(uint8_t power);
    uint8_t getMaximumPower();
    void setFanSpeed(uint8_t speed);
    void setPIDTuning(double kp, double ki, double kd);
    void pause();
    void resume();
    int16_t getTemperature();
    uint8_t getPower();
    uint8_t getFanSpeed();
    uint8_t getIndex();

protected:

private:
    uint8_t calculateHeaterPower();

    static uint8_t activeHeaters; // a static counter to establish how many heaters are active in non-PWM mode
    TemperatureSensor *sensorHeater;
    Heater *heater;
    Fan *fan;
    double targetTemperature, currentTemperature, power;
    uint8_t maxPower; // maximum power applied to heater (0-255)
    uint8_t index; // the id/number of the plate
    PID *pid; // pointer to PID controller
    bool paused; // flag indicating if the plate is in paused mode
};

#endif /* PLATE_H_ */

