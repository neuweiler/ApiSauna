/*
 * Fan.cpp
 *
 * Class which controls a fan via PWM.
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

#include "Fan.h"

bool Fan::pwmInitialized = false;

/**
 * Constructor.
 */
Fan::Fan() {
	controlPin = 0;
	minimumSpeed = 0;
	speed = 0;
}

Fan::~Fan() {
	logger.debug(F("Fan %d destroyed"), controlPin);
}

void Fan::initialize(uint8_t controlPin, uint8_t minimumSpeed) {
	logger.info(F("initializing fan on pin %d (%x)"), controlPin, this);

	initializePWM();

	this->minimumSpeed = minimumSpeed;
	this->controlPin = controlPin;
	pinMode(controlPin, OUTPUT);
	setSpeed(0);
}

/**
 * Configure the PWM ports and adjust the timers so the PWM frequency is usable to control
 * PWM fans and the heaters.
 *
 * The default PWM frequency is 490 Hz for all pins except pin 13 and 4, which use 980 Hz
 * The AT Mega 2560 provides the following timers:
 *  #0 8bit  (pin 13, 4)      : reserved (affects timing functions like delay() and millis())
 *  #1 16bit (pin 11, 12)     : heater #1 and #2
 *  #2 8bit  (pin 9, 10)      : reserved (used for tone())
 *  #3 16bit (pin 2, 3, 5)    : heater #3 - #4, vaporizer
 *  #4 16bit (pin 6, 7, 8)    : humidifier fan, fans #1 - #2
 *  #5 16bit (pin 44, 45, 46) : fans #3 - #4, reserve
 */
void Fan::initializePWM() {
	if (!pwmInitialized) {
		logger.info(F("initializing PWM timers"));
		// set timer 4 to 31kHz for controlling PWM fans
		TCCR4B &= ~7;
		TCCR4B |= 1;
		// set timer 5 to 31kHz for controlling PWM fans
		TCCR5B &= ~7;
		TCCR5B |= 1;

		pwmInitialized = true;
	}
}

/**
 * Set the speed of the fan (value 0-255)
 */
void Fan::setSpeed(uint8_t speed) {
	logger.debug(F("setting speed of fan on pin %d to %d"), controlPin, speed);
	if (controlPin > 0) {
		this->speed = constrain(speed, minimumSpeed, 255);
		analogWrite(controlPin, this->speed);
	}
}

/**
 * Get the current speed (0-255)
 */
uint8_t Fan::getSpeed() {
	return speed;
}
