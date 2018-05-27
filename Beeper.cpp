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

Beeper::Beeper() : Device()
{
    lastState = Status::init;
    numberOfBeeps = 0;
    soundOn = false;
}

void Beeper::initialize()
{
    Device::initialize();
    uint8_t pin = Configuration::getIO()->beeper;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void Beeper::process()
{
    Device::process();
    Status::SystemState state = Status::getInstance()->getSystemState();

    if (lastState != state) {
        switch (state) {
        case Status::overtemp:
            numberOfBeeps = -1; // unlimited
            break;
        case Status::error:
            numberOfBeeps = 20;
            break;
        default:
            numberOfBeeps = 2;
            break;
        }
        lastState = state;
    }
    beep();
}

void Beeper::beep()
{
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
