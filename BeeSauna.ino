/*
 * BeeSauna.cpp
 *
 * Main part of the bee sauna control software. It initializes all devices
 * and controls the main loop.
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

#include "BeeSauna.h"

Controller controller;
HID hid;

void delayStart()
{
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
}

/**
 * Configure the PWM ports and adjust the timers so the PWM frequency is usable to control
 * PWM fans and the heaters.
 *
 * The default PWM frequency is 490 Hz for all pins except pin 13 and 4, which use 980 Hz
 * The AT Mega 2560 provides the following timers:
 *  #0 8bit  (pin 13, 4)      : reserved (affects timing functions like delay() and millis())
 *  #1 16bit (pin 12, 11)     : heater #1 and #2
 *  #2 8bit  (pin 10, 9)      : reserved (used for tone())
 *  #3 16bit (pin 5, 3, 2)    : heater #3 - #5
 *  #4 16bit (pin 8, 7, 6)    : fans #1 - #3
 *  #5 16bit (pin 44, 45, 46) : fans #4, #5 and humidifier fan
 */
void configurePwm()
{
    // prepare pins
    pinMode(2, OUTPUT);
    analogWrite(2, 0);
    pinMode(3, OUTPUT);
    analogWrite(3, 0);
    pinMode(5, OUTPUT);
    analogWrite(5, 0);
    pinMode(6, OUTPUT);
    analogWrite(6, 0);
    pinMode(7, OUTPUT);
    analogWrite(7, 0);
    pinMode(8, OUTPUT);
    analogWrite(8, 0);
    pinMode(11, OUTPUT);
    analogWrite(11, 0);
    pinMode(12, OUTPUT);
    analogWrite(12, 0);
    pinMode(44, OUTPUT);
    analogWrite(44, 0);
    pinMode(45, OUTPUT);
    analogWrite(45, 0);
    pinMode(46, OUTPUT);
    analogWrite(46, 0);

    // set timer 4+5 to 31kHz for controlling PWM fans
    TCCR4B &= ~7;
    TCCR4B |= 1;
    TCCR5B &= ~7;
    TCCR5B |= 1;

    // set timer 1+3 to 31kHz for controlling the heaters
    TCCR1B &= ~7;
    TCCR1B |= 1;
    TCCR3B &= ~7;
    TCCR3B |= 1;
}

void setup()
{
    Serial.begin(CFG_SERIAL_SPEED);
    delayStart();
    Serial.println(CFG_VERSION);

    configurePwm();
    controller.init();
    hid.init(&controller);

    status.setSystemState(Status::ready);
}

void loop()
{
    controller.loop();
    hid.loop();
}
