/*
 * Fan.cpp
 *
 * Class which controls a fan via PWM.
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

#include "Fan.h"

/**
 * Constructor.
 */
Fan::Fan() : Fan::Fan(0)
{
}

/**
 * Constructor, specify PWM pin to control the speed of the fan.
 */
Fan::Fan(uint8_t controlPin)
{
    setControlPin(controlPin);
}

Fan::~Fan()
{
    setSpeed(0);
    this->controlPin = 0;
}

/**
 * Set the control pin for the fan
 */
void Fan::setControlPin(uint8_t controlPin) {
    this->controlPin = controlPin;
    pinMode(controlPin, OUTPUT);
    setSpeed(0);
}

/**
 * Set the speed of the fan (value 0-255)
 */
void Fan::setSpeed(uint8_t speed)
{
    if ((controlPin >= 2 && controlPin <= 13) || (controlPin >= 44 && controlPin <= 46)) {
        this->speed = constrain(speed, 0, 255);
        analogWrite(controlPin, this->speed);
    }
}

/**
 * Get the current speed (0-255)
 */
uint8_t Fan::getSpeed()
{
    return speed;
}
