/*
 * ApiSauna.ino
 *
 * The main application, hands over control to the Controller class.
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

#include "ApiSauna.h"

/**
 * The initial setup function called after device reset/power-up
 */
void setup() {
	Serial.begin(CFG_SERIAL_SPEED);
	Serial.println(CFG_VERSION);

	serialConsole.initialize();

	configuration.load();

	hive.initialize();
	humidifier.initialize();
	hid.initialize();
}

/**
 * The main program loop
 */
void loop() {
	static uint8_t tickCounter = 0;

	if (tickCounter++ > 7) {
		eventHandler.publish(EventListener::PROCESS);
		tickCounter = 0;
	}

	eventHandler.publish(EventListener::PROCESS_INPUT);

	delay(CFG_LOOP_DELAY);
}
