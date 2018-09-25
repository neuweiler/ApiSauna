/*
 * Beeper.cpp
 *
 * Makes sounds depending on system state.
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

#include "Beeper.h"

Beeper::Beeper() :
        Device()
{
    numberOfBeeps = 0;
    soundOn = false;
}

/**
 * Initialize the beeper
 */
void Beeper::initialize()
{
    Device::initialize();
    uint8_t pin = Configuration::getIO()->beeper;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

/**
 * Process the pending beep requests and turn on/off piezo device
 */
void Beeper::process()
{
    Device::process();
    if (soundOn) {
        soundOn = false;
        if (numberOfBeeps != -1) {
            numberOfBeeps--;
        }
    } else if (numberOfBeeps != 0) {
        soundOn = true;
    }

    analogWrite(Configuration::getIO()->beeper, (soundOn ? 20 : 0));
}

/**
 * Request a specified amount of beeps
 */
void Beeper::beep(int8_t numberOfBeeps)
{
    this->numberOfBeeps = numberOfBeeps;
}

/**
 * Produce a short click, e.g. for feedback to interactive input
 */
void Beeper::click()
{
    analogWrite(Configuration::getIO()->beeper, 30);
    delay(20);
    analogWrite(Configuration::getIO()->beeper, 0);
}
