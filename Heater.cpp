/*
 * Heater.cpp
 *
 * Controls the power of the heater device.
 *
 Copyright (c) 2017 Michael Neuweiler

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
z
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

#include "Heater.h"

/**
 * Constructor
 */
Heater::Heater() : Heater::Heater(0)
{
}

/**
 * Constructor, specify PWM pin to control the power of the heater.
 */
Heater::Heater(uint8_t index)
{
    this->index = constrain(index, 0, CFG_MAX_NUMBER_PLATES - 1);
    pinMode(Configuration::getIO()->heater[this->index], OUTPUT);
    setPower(0);
}

Heater::~Heater()
{
    setPower(0);
}

/**
 * Set the power of the heater (value 0-255).
 * The value is automatically limited to the configured max HeaterPower
 */
void Heater::setPower(uint8_t power)
{
    this->power = power;
    analogWrite(Configuration::getIO()->heater[index], this->power);
}

uint8_t Heater::getPower() {
    return power;
}
