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

#ifndef CFG_USE_PWM
    uint8_t Plate::activeHeaters = 0;
#endif

PlateConfig::PlateConfig() :
        PlateConfig::PlateConfig(0, 0, 0, 0)
{
}

PlateConfig::PlateConfig(uint8_t id, uint64_t addressHeater, uint8_t heaterPin, uint8_t fanPin)
{
    this->sensorAddressHeater.value = addressHeater;
    this->heaterPin = heaterPin;
    this->fanPin = fanPin;
    this->id = id;
}

Plate::Plate()
{
    currentTemperature = 0;
    targetTemperature = 0;
    maxPower = CFG_MAX_HEATER_POWER;
    pid = NULL;
    power = 0;
    id = 0;
}

Plate::Plate(PlateConfig *config) :
        Plate::Plate()
{
    sensorHeater.setAddress(config->sensorAddressHeater);
    heater.setControlPin(config->heaterPin);
    fan.setControlPin(config->fanPin);
    this->id = config->id;
}

Plate::~Plate()
{
    if (pid) {
        delete pid;
        pid = NULL;
    }
}

/**
 * Set the maximum temperature of the plate (in 0.1 deg C)
 */
void Plate::setTargetTemperature(int16_t temperature)
{
    targetTemperature = temperature;
}

int16_t Plate::getTargetTemperature()
{
    return targetTemperature;
}

/**
 * Set the maximum temperature of the plate (0-255)
 */
void Plate::setMaximumPower(uint8_t power)
{
    this->maxPower = constrain(power, (double) 0, CFG_MAX_HEATER_POWER);
    checkPid();
    pid->SetOutputLimits(0, this->maxPower);
}

uint8_t Plate::getMaximumPower()
{
    return maxPower;
}

/**
 * Set the plate's fan speed (0-255)
 */
void Plate::setFanSpeed(uint8_t speed)
{
    fan.setSpeed(speed);
}

/**
 * Set the tuning parameters of the PID controller for the plate temperature
 */
void Plate::setPIDTuning(double kp, double ki, double kd)
{
    checkPid();
    pid->SetTunings(kp, ki, kd);
}

/**
 * Get the temperature of the plate in 0.1 deg C
 */
int16_t Plate::getTemperature()
{
    return sensorHeater.getTemperatureCelsius();
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
 * Get the plate's id.
 */
uint8_t Plate::getId()
{
    return id;
}

void Plate::checkPid()
{
    // need to do late init because if placed into list, object is duplicated and addresses of PID variables shift again
    if (!pid) {
        pid = new PID(&currentTemperature, &power, &targetTemperature, 0, 0, 0, DIRECT);
        pid->SetOutputLimits(0, maxPower);
        pid->SetSampleTime(CFG_LOOP_DELAY);
        pid->SetMode(AUTOMATIC);
    }
}

uint8_t Plate::calculateHeaterPower()
{
    double oldPower = power;

    checkPid();
    pid->Compute();
    if (currentTemperature > CFG_PLATE_OVER_TEMPERATURE) {
        Logger::error("!!!!! ALERT !!!!! Plate %d is over-heating !!!", id);
        status.setSystemState(Status::overtemp);
        power = 0;
    }
#ifdef CFG_USE_PWM
    return constrain(power, (double )0, maxPower);
#else
    if ((power == CFG_MAX_HEATER_POWER) && (activeHeaters < CFG_MAX_CONCURRENT_HEATERS)) {
        activeHeaters++;
        return 255;
    } else {
        if (oldPower > 0 && activeHeaters > 0) {
            activeHeaters--;
        }
        return 0;
    }
#endif

}

/**
 * Method called for each plate in the main loop.
 * It re-calculates the power and fan speed according to current
 * settings and sensor data.
 */
void Plate::loop()
{
    sensorHeater.retrieveData();
    currentTemperature = sensorHeater.getTemperatureCelsius();

    heater.setPower(calculateHeaterPower());
}
