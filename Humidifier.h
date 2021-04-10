/*
 * Humidifier.h
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

#ifndef HUMIDIFIER_H_
#define HUMIDIFIER_H_

#include "EventHandler.h"
#include "HumiditySensor.h"
#include "Fan.h"
#include "Program.h"

class Humidifier: EventListener {
public:
	Humidifier();
	virtual ~Humidifier();
	void initialize();
	void handleEvent(Event event, va_list args);

private:
	void process();
	void enableVaporizer(bool on);
	void programChange(const Program *program);
	void setFanSpeed(uint8_t speed);

	HumiditySensor sensor;
	Fan fan;
	uint8_t maximumHumidity;
	uint8_t minimumHumidity;
	uint8_t fanSpeed; // the program's desired fan speed for the humidifier
	uint32_t fanTimestamp;
	bool running; // indicates if the program is running
	bool paused; // indicates if the program execution is paused
	bool alert; // indicates if we're in a temperature alert condition
	StatusHumidity status;
};

extern Humidifier humidifier;

#endif /* HUMIDIFIER_H_ */
