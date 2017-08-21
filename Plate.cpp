/*
 * Plate.cpp
 *
 * Class which controls a single heating palte which consists of a heating element,
 * temperature sensor (to measure the plate's heat) and a fan to distribute the heat.
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

#include "Plate.h"

PlateConfig::PlateConfig() {
    sensorAddress.value = 0;
    heaterPin = 0;
    fanPin = 0;
}

PlateConfig::PlateConfig(uint64_t address, uint8_t heaterPin, uint8_t fanPin)
{
    this->sensorAddress.value = address;
    this->heaterPin = heaterPin;
    this->fanPin = fanPin;
}

Plate::Plate()
{
    //TODO replace testing values with real ones
    maxTemperature = 300; // 70.0 deg C
    deratingTemperature = 250; // 50.0 deg C
    maxPower = 255; // 100%
}

Plate::Plate(SensorAddress sensorAddress, uint8_t heaterPin, uint8_t fanPin)
{
    temperatureSensor.setAddress(sensorAddress);
    heater.setControlPin(heaterPin);
    fan.setControlPin(fanPin);

    maxTemperature = 300; // 70.0 deg C
    deratingTemperature = 250; // 50.0 deg C
    maxPower = 255; // 100%
}

Plate::~Plate()
{
    maxTemperature = 0;
    deratingTemperature = 0;
    maxPower = 0;
}

/**
 * Set the maximum temperature of the plate (in 0.1 deg C)
 * Automatically adjusts the derating temperature if necessary
 */
void Plate::setMaximumTemperature(int16_t temperature)
{
    maxTemperature = temperature;
    deratingTemperature = min(temperature - 1, deratingTemperature); // max and derating must never be equal (DIVby0)
}

/**
 * Set the temperature of the plate where derating begins (in 0.1 deg C)
 * Automatically adjusts maximum temperature if necessary
 */
void Plate::setDeratingTemperature(int16_t temperature)
{
    deratingTemperature = temperature;
    maxTemperature = max(temperature + 1, maxTemperature); // max and hysterisis must never be equal (DIVby0)
}

/**
 * Set the maximum temperature of the plate (0-255)
 */
void Plate::setMaximumPower(uint8_t power)
{
    this->maxPower = power;
}

/**
 * Set the plate's fan speed (0-255)
 */
void Plate::setFanSpeed(uint8_t speed)
{
    fan.setSpeed(speed);
}

/**
 * Get the temperature of the plate in 0.1 deg C
 */
int16_t Plate::getTemperature()
{
    return temperatureSensor.getTemperatureCelsius();
}

/**
 * Get the applied power of the heater (0-255)
 */
uint8_t Plate::getPower()
{
    return heater.getPower();
}

/**
 * Get the current fan speed.
 */
uint8_t Plate::getFanSpeed()
{
    return fan.getSpeed();
}

/**
 * Method called for each plate in the main loop.
 * It re-calculates the power and fan speed according to current
 * settings and sensor data.
 */
void Plate::loop()
{
    temperatureSensor.retrieveData(); // get the data from the sensor
    int16_t temp = temperatureSensor.getTemperatureCelsius();
    uint8_t power = 0;

    if (temp < maxTemperature) {
        if (temp < deratingTemperature) {
            power = maxPower; // full power
        } else {
            power = maxPower * (maxTemperature - temp) / (maxTemperature - deratingTemperature); // derating
        }
    }
    heater.setPower(power);
}
