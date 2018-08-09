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

Humidifier::Humidifier() :
        Device()
{
    maximumHumidity = 0;
    minimumHumidity = 0;
    humidity = 0;
    fanSpeed = 0;
    temperature = 0;
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

void Humidifier::setFanSpeed(uint8_t speed)
{
    fanSpeed = speed;
}

uint8_t Humidifier::getFanSpeed()
{
    return fan.getSpeed();
}

uint8_t Humidifier::getHumidity()
{
    return humidity;
}

int16_t Humidifier::getTemperature()
{
    return temperature;
}

Vaporizer::Mode Humidifier::getVaporizerMode()
{
    return vaporizer.getMode();
}

void Humidifier::initialize()
{
    Device::initialize();
    sensor.init();
    fan.setControlPin(Configuration::getIO()->humidifierFan);
}

void Humidifier::process()
{
    Device::process();
    Status *status = Status::getInstance();
    humidity = sensor.getRelativeHumidity();
    temperature = sensor.getTemperature();

    if (humidity != 0 && humidity < minimumHumidity) {
        fan.setSpeed(fanSpeed);
        vaporizer.setMode(Vaporizer::ON);
        status->vaporizerEnabled = true;
        status->fanSpeedHumidifier = fanSpeed;
        status->fanTimeHumidifier = 0;
    }

    if (humidity >= maximumHumidity) {
        vaporizer.setMode(Vaporizer::OFF);
        status->vaporizerEnabled = false;

        if (status->fanTimeHumidifier == 0) {
            status->fanTimeHumidifier = millis();
        }
        if ((millis() - status->fanTimeHumidifier) > 60000 * Configuration::getParams()->humidifierFanDryTime) {
            fan.setSpeed(0);
            status->fanSpeedHumidifier = 0;
        }
    }

    status->humidity = humidity;
    status->temperatureHumidifier = temperature;

}
