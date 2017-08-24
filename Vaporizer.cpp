/*
 * Vaporizer.cpp
 *
 * Class which controls the vaporizer of the humidifier.
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

#include "Vaporizer.h"

Vaporizer::Vaporizer()
{
    controlPin = 0;
    mode = OFF;
}

Vaporizer::Vaporizer(uint8_t controlPin)
{
    setControlPin(controlPin);
}

Vaporizer::~Vaporizer()
{
    setMode(OFF);
    controlPin = 0;
}

void Vaporizer::setControlPin(uint8_t controlPin)
{
    this->controlPin = controlPin;
    pinMode(controlPin, OUTPUT);
    setMode(OFF);
}

void Vaporizer::setMode(Mode mode)
{
    if (controlPin != 0) {
        this->mode = mode;
        digitalWrite(controlPin, (mode == ON));
    }
}

Vaporizer::Mode Vaporizer::getMode()
{
    return mode;
}