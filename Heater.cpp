/*
 * Heater.cpp
 *
 * Controls the power of the heater device.
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

#include "Heater.h"

/**
 * Constructor
 */
Heater::Heater() {
	controlPin = 0;
	power = 0;
}

Heater::~Heater() {
	logger.debug(F("Heater %d destroyed"), controlPin);
}

/**
 * specify PWM pin to control the power of the heater.
 */
void Heater::initialize(uint8_t controlPin) {
	logger.info(F("initializing heater on pin %d (%x)"), controlPin, this);

	this->controlPin = controlPin;
	pinMode(controlPin, OUTPUT);
	setPower(0);
}

/**
 * Set the power of the heater (value 0-255).
 */
void Heater::setPower(uint8_t power) {
	logger.debug(F("setting power of heater on pin %d to %d"), controlPin, power);
	if (controlPin > 0) {
		this->power = power;
		analogWrite(controlPin, this->power);
	}
}

uint8_t Heater::getPower() {
	return power;
}
