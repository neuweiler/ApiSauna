/*
 * Humidifier.cpp
 *
 * Measures the humidity of the hive and controls the vaporizer and
 * its fan.
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

#include "Humidifier.h"

Humidifier humidifier;

Humidifier::Humidifier() {
	maximumHumidity = 0;
	minimumHumidity = 0;
	fanSpeed = 0;
	fanTimestamp = 0;
	running = false;
	paused = false;
	alert = false;

	status.fanSpeed = 0;
	status.humidity = 0;
	status.temperature = 0;
	status.vaporizer = false;
}

Humidifier::~Humidifier() {

}

void Humidifier::handleEvent(Event event, ...) {
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
		running = false;
		paused = false;
		break;
	case PROGRAM_PAUSE:
		paused = true;
		break;
	case PROGRAM_RESUME:
		paused = false;
		break;
	case TEMPERATURE_ALERT:
		alert = true;
		break;
	case TEMPERATURE_NORMAL:
		alert = false;
	}
}

/**
 * Initialize the humidifier and its devices
 */
void Humidifier::initialize() {
	sensor.initialize();
	fan.initialize(configuration.getIO()->humidifierFan, 0);

	pinMode(configuration.getIO()->vaporizer, OUTPUT);
	enableVaporizer(false);

	paused = false;
	running = false;

	eventHandler.subscribe(this);
}

/**
 * Read the humidity and activate/deactivate the vaporizer/fan.
 */
void Humidifier::process() {
	uint8_t humidity = sensor.getRelativeHumidity();

	if (humidity != 0 && humidity < minimumHumidity) {
//TODO if fanSpeed < 240, give it a kick of 240 to break free
		fan.setSpeed(fanSpeed);
		enableVaporizer(true);
		fanTimestamp = 0;
	}

	if (humidity >= maximumHumidity || paused || !running) {
		enableVaporizer(false);
		if (fanTimestamp == 0) {
			fanTimestamp = millis();
		}
	}

	// let the fan run longer than the vaporizer to let it dry
	if (fanTimestamp != 0 && (millis() - fanTimestamp) > 60000 * configuration.getParams()->humidifierFanDryTime) {
		fan.setSpeed(0);
		fanTimestamp = 0;
	}

	status.humidity = humidity;
	status.temperature = sensor.getTemperature();
	eventHandler.publish(EventListener::STATUS_HUMIDITY, status);
}

void Humidifier::programChange(const Program &program) {
	maximumHumidity = program.humidityMaximum;
	minimumHumidity = program.humidityMinimum;
	fanSpeed = program.fanSpeedHumidifier;
	running = program.running;
}

void Humidifier::enableVaporizer(bool enable) {
	if (configuration.getIO()->vaporizer != 0) {
		digitalWrite(configuration.getIO()->vaporizer, enable);
	}
	status.vaporizer = enable;
}

void Humidifier::setFanSpeed(uint8_t speed) {
	fan.setSpeed(speed);
	status.fanSpeed = speed;
}
