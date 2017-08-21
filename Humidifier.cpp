/*
 * Humidifier.cpp
 *
 * Measures the humidity of the hive and controls the evaporator and
 * its fan.
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

#include "Humidifier.h"

Humidifier::Humidifier()
{
    maximumHumidity = 0;
    minimumHumidity = 0;
    humidity = 0;
}

Humidifier::Humidifier(uint8_t sensorPin, uint8_t fanPin, uint8_t evaporatorPin)
{
    sensor.setControlPin(sensorPin);
    fan.setControlPin(fanPin);
    evaporator.setControlPin(evaporatorPin);
    maximumHumidity = 0;
    minimumHumidity = 0;
    humidity = 0;
}

Humidifier::~Humidifier()
{
    minimumHumidity = 0;
    maximumHumidity = 0;
    humidity = 0;
}

void Humidifier::setSensorPin(uint8_t sensorPin)
{
    sensor.setControlPin(sensorPin);
}

void Humidifier::setFanPin(uint8_t fanPin)
{
    fan.setControlPin(fanPin);
}

void Humidifier::setEvaporatorPin(uint8_t evaproatorPin)
{
    evaporator.setControlPin(evaproatorPin);
}

void Humidifier::setMaxHumidity(uint8_t maxHumidity)
{
    this->maximumHumidity = maxHumidity;
}

uint8_t Humidifier::getMaxHumidity()
{
    return maximumHumidity;
}

void Humidifier::setMinHumidity(uint8_t minHumidity)
{
    this->minimumHumidity = minHumidity;
}

uint8_t Humidifier::getMinHumidity()
{
    return minimumHumidity;
}

uint8_t Humidifier::getHumidity()
{
    return humidity;
}

uint8_t Humidifier::getFanSpeed()
{
    return fan.getSpeed();
}

Evaporator::Mode Humidifier::getEvaporatorMode()
{
    return evaporator.getMode();
}

void Humidifier::loop()
{
    humidity = sensor.getRelativeHumidity();

    if (humidity < minimumHumidity) {
        fan.setSpeed(127);
        evaporator.setMode(Evaporator::ON);
    }

    if (humidity > maximumHumidity) {
        fan.setSpeed(0);
        evaporator.setMode(Evaporator::OFF);
    }
}
