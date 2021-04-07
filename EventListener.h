/*
 * EventListener.h
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

#ifndef EVENTLISTENER_H_
#define EVENTLISTENER_H_

#include <Arduino.h>
#include "config.h"

class EventListener {
public:
	struct StatusHumidity {
		uint8_t humidity;
		int16_t temperature;
		bool vaporizer;
		uint8_t fanSpeed;
	};

	struct StatusZone {
		uint8_t id;
		int16_t temperatureTarget;
		int16_t temperatureActual;
	};

	struct StatusPlate {
		uint8_t id;
		int16_t temperatureTarget;
		int16_t temperatureActual;
		uint8_t power;
		uint8_t fanSpeed;
	};

	enum Event {
		PROGRAM_PREHEAT = 1 << 0, // param: Program
		PROGRAM_RUN = 1 << 1, // param: Program
		PROGRAM_STOP = 1 << 2,
		PROGRAM_PAUSE = 1 << 3,
		PROGRAM_RESUME = 1 << 4,
		PROGRAM_UPDATE = 1 << 5, // param: Program
		TEMPERATURE_NORMAL = 1 << 6,
		TEMPERATURE_HIGH = 1 << 7,
		TEMPERATURE_ALERT = 1 << 8,
		STATUS_HUMIDITY = 1 << 9, // param: StatusHumidity
		STATUS_ZONE = 1 << 10, // param: StatusZone
		STATUS_PLATE = 1 << 11, // param: StatusPlate
		PROCESS_INPUT = 1 << 13, // called every 100ms
		PROCESS = 1 << 14, // called every 1000ms
		ERROR = 1 << 15,
	};

	virtual void handleEvent(Event event, va_list args);
};

#endif /* EVENTLISTENER_H_ */
