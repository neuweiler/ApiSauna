/*
 * Humidifier.cpp
 *
 * Measures the humidity of the hive and controls the vaporizer and
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
    fanSpeed = 0;
}

Humidifier::Humidifier(uint8_t sensorPin, uint8_t fanPin, uint8_t vaporizerPin)
{
    sensor.setControlPin(sensorPin);
    fan.setControlPin(fanPin);
    vaporizer.setControlPin(vaporizerPin);
    maximumHumidity = 0;
    minimumHumidity = 0;
    humidity = 0;
    fanSpeed = 0;
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

void Humidifier::setVaporizerPin(uint8_t vaporizerPin)
{
    vaporizer.setControlPin(vaporizerPin);
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

void Humidifier::setFanSpeed(uint8_t speed)
{
    fanSpeed = speed;
}

uint8_t Humidifier::getFanSpeed()
{
    return fan.getSpeed();
}

Vaporizer::Mode Humidifier::getVaporizerMode()
{
    return vaporizer.getMode();
}

void Humidifier::loop()
{
    humidity = sensor.getRelativeHumidity();

    if (humidity != 0 && humidity < minimumHumidity) {
        fan.setSpeed(fanSpeed);
        vaporizer.setMode(Vaporizer::ON);
    }

    if (humidity >= maximumHumidity) {
        fan.setSpeed(0);
        vaporizer.setMode(Vaporizer::OFF);
    }
}
