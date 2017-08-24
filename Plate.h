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

#include "Fan.h"
#include "Heater.h"
#include "TemperatureSensor.h"
#include "Status.h"

class PlateConfig
{
public:
    PlateConfig();
    PlateConfig(uint8_t id, uint64_t address, uint8_t heaterPin, uint8_t fanPin);
    SensorAddress sensorAddress;
    uint8_t heaterPin;
    uint8_t fanPin;
    uint8_t id;
};

class Plate
{
public:
    Plate();
    Plate(uint8_t id, SensorAddress sensorAddress, uint8_t heaterPin, uint8_t fanPin);
    virtual ~Plate();
    void setMaximumTemperature(int16_t temperature);
    int16_t getMaximumTemperature();
    void setDeratingDelta(int16_t temperature);
    void setMaximumPower(uint8_t power);
    uint8_t getMaximumPower();
    void setFanSpeed(uint8_t speed);
    int16_t getTemperature();
    uint8_t getPower();
    uint8_t getFanSpeed();
    uint8_t getId();
    void loop();

protected:

private:
    TemperatureSensor temperatureSensor;
    Heater heater;
    Fan fan;
    int16_t maxTemperature; // maximum temperature of the plate (in 0.1 deg C)
    int16_t deratingDelta; // temperature delta against max temp where derating begins (in 0.1 deg C)
    uint8_t maxPower; // maximum power applied to heater (0-255)
    uint8_t id; // the id/number of the plate
};

#endif /* PLATE_H_ */

