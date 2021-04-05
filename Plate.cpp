/*
 * Plate.cpp
 *
 * Class which controls a single heating palte which consists of a heating element,
 * temperature sensor (to measure the plate's heat) and a fan to distribute the heat.
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

#include "Plate.h"

uint8_t Plate::activeHeaters = 0;

Plate::Plate()
{
    number = 0;
    currentTemperature = 0.0;
    targetTemperature = 0.0;
    power = 0.0;
    pid = NULL;
    paused = false;
    running = false;
}

Plate::Plate(uint8_t number) : Plate()
{
    this->number = number;
}

Plate::~Plate()
{
    if (pid != NULL) {
        delete pid;
        pid = NULL;
    }
}

void Plate::handleEvent(Event event, ...)
{
    switch (event) {
    case PROCESS:
        process();
        break;
    case PROGRAM_START:
    case PROGRAM_UPDATE:
        va_list args;
        va_start(args, event);
        programChange(va_arg(args, Program));
        va_end(args);
        break;
    case PROGRAM_STOP:
    case TEMPERATURE_ALERT:
        running = false;
        paused = false;
        break;
    case TEMPERATURE_HIGH:
    case PROGRAM_PAUSE:
        paused = true;
        break;
    case PROGRAM_RESUME:
    case TEMPERATURE_NORMAL:
        paused = false;
        break;
    }
}

void Plate::initialize()
{
    ConfigurationIO *configIo = configuration.getIO();

    sensor.setAddress(configuration.getSensor()->addressPlate[number]);
    fan.initialize(configIo->fan[number], configuration.getParams()->minFanSpeed);
    heater.begin(configIo->heater[number]);

    if (pid != NULL) {
        delete pid;
    }
    pid = new PID(&currentTemperature, &power, &targetTemperature, 0, 0, 0, DIRECT);
    pid->SetOutputLimits(0, configuration.getParams()->maxHeaterPower);
    pid->SetSampleTime(CFG_LOOP_DELAY);
    pid->SetMode(AUTOMATIC);

    paused = false;
    running = false;

    eventHandler.subscribe(this);
}

/**
 * Method called for each plate in the main loop.
 * It re-calculates the power and fan speed according to current
 * settings and sensor data.
 */
void Plate::process()
{
    sensor.retrieveData();
    currentTemperature = sensor.getTemperatureCelsius();

    if (paused || !running) {
        heater.setPower(0);
        if (!running) {
            fan.setSpeed(0);
        }
    } else {
        heater.setPower(calculateHeaterPower());
    }
}

void Plate::programChange(const Program& program)
{
    pid->SetOutputLimits(0, configuration.getParams()->maxHeaterPower);
    pid->SetTunings(program.plateKp, program.plateKi, program.plateKd);
    fan.setSpeed(program.preHeat ? program.fanSpeedPreHeat : program.fanSpeed);
    running = program.running;
}

/**
 * Set the desired temperature of the plate (in 0.1 deg C)
 */
void Plate::setTargetTemperature(int16_t temperature)
{
    targetTemperature = temperature;
}

/**
 * Update the plate's PID data and derive the power level to command (0-255 for PWM, 0 / 255 for non-PWM).
 * In non-PWM mode, the number of concurrently active plates is also limited to the configured amount.
 *
 */
uint8_t Plate::calculateHeaterPower()
{
    ConfigurationParams *params = configuration.getParams();

    pid->Compute(); // updates power

    if (Logger::isDebug()) {
        Logger::debug(F("calculated power for plate %d: %d"), number, power);
    }

    if (currentTemperature > params->plateOverTemp) {
        Logger::error(F("ALERT !!! Plate %d is over-heating !!!"), number);
        eventHandler.publish(TEMPERATURE_ALERT);
        return 0;
    }

    if (params->usePWM) {
        return constrain(power, (double )0, params->maxHeaterPower);
    } else {
        bool isOn = heater.getPower() > 0;
        if ((power >= params->maxHeaterPower / 2) && (activeHeaters < params->maxConcurrentHeaters)) {
            if (!isOn) {
                activeHeaters++;
            }
            return 255;
        } else {
            if (isOn && activeHeaters > 0) {
                activeHeaters--;
            }
            return 0;
        }
    }
}
