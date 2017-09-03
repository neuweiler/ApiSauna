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

Beeper::Beeper()
{
    controlPin = 0;
    lastState = Status::init;
    numberOfBeeps = 0;
    soundOn = false;
}

Beeper::~Beeper()
{
}

void Beeper::setControlPin(uint8_t controlPin)
{
    this->controlPin = controlPin;
    pinMode(controlPin, OUTPUT);
    digitalWrite(controlPin, 0);
}

void Beeper::loop()
{
    Status::SystemState state = status.getSystemState();

    if (lastState != state) {
        switch (status.getSystemState()) {
        case Status::overtemp:
            numberOfBeeps = -1; // unlimited
            break;
        case Status::error:
            break;
        default:
            break;
        }
        lastState = state;
    }

    if (soundOn) {
        //TODO turn sound off
        Serial.println("Beep off");
        soundOn = false;
        if (numberOfBeeps != -1) {
            numberOfBeeps--;
        }
    } else if (numberOfBeeps != 0) {
        //TODO turn sound on
        Serial.println("BEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEP !!!!!!!!!!!!!!!!!!!!!!!!!");
        soundOn = true;
    }
}
