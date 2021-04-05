/*
 * Hive.cpp
 *
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

#include "Hive.h"

Hive hive;

Hive::Hive() {
	tickCounter = 0;
}

Hive::~Hive() {
}

void Hive::handleEvent(Event event, ...) {
	switch (event) {
	case PROCESS:
		process();
		break;
	case PROGRAM_START:
	case PROGRAM_UPDATE:
		va_list args;
		va_start(args, event);
		handleProgramChange(va_arg(args, Program));
		//    startTime = millis();
		//    status.setSystemState(Status::running);
		//    eventHandler->sendEvent(updateProgram);

		va_end(args);
		break;
	case PROGRAM_STOP:
		break;
	}
}

void Hive::initialize() {
	logger.info(F("initializing hive"));

	uint8_t heaterRelay = configuration.getIO()->heaterRelay;
	pinMode(heaterRelay, OUTPUT);
	digitalWrite(heaterRelay, LOW);

	HiveFactory factory;
	factory.create();

	eventHandler.subscribe(this);
}

void Hive::process() {
	if (tickCounter++ < 10) {
		return;
	}
	tickCounter = 0;

	/*
	 Program *runningProgram = programHandler->getRunningProgram();
	 uint32_t timeRemaining = programHandler->calculateTimeRemaining();

	 if (actualTemperature > configuration.getParams()->hiveOverTemp) {
	 logger.error(F("ALERT - OVER-TEMPERATURE IN HIVE ! Trying to recover, please open the cover to help cool down the hive!"));
	 status.setSystemState(Status::overtemp);
	 status.errorCode = Status::overtempHive;
	 }
	 if (status.getSystemState() == Status::overtemp && actualTemperature < configuration.getParams()->hiveMaxTemp) {
	 logger.info(F("recovered from over-temperature, shutting down."));
	 programHandler->stop();
	 }

	 if (status.getSystemState() == Status::running && timeRemaining < 2) {
	 logger.info(F("program finished."));
	 programHandler->stop();
	 }

	 if (status.getSystemState() == Status::preHeat && runningProgram
	 && (timeRemaining == 0
	 || (actualTemperature >= runningProgram->temperaturePreHeat && timeRemaining < programHandler->calculateTimeRunning()))) {
	 programHandler->switchToRunning();
	 }*/

}

/*
 void Hive::addTime(uint16_t duration)
 {
 Program *clone = new Program();
 clone->duration = duration;
 clone->fanSpeed = runningProgram->fanSpeed;
 clone->fanSpeedHumidifier = runningProgram->fanSpeedHumidifier;
 clone->hiveKp = runningProgram->hiveKp;
 clone->hiveKi = runningProgram->hiveKi;
 clone->hiveKd = runningProgram->hiveKd;
 clone->humidityMinimum = runningProgram->humidityMinimum;
 clone->humidityMaximum = runningProgram->humidityMaximum;
 strcpy(clone->name, runningProgram->name);
 clone->plateKp = runningProgram->plateKp;
 clone->plateKi = runningProgram->plateKi;
 clone->plateKd = runningProgram->plateKd;
 clone->temperatureHive = runningProgram->temperatureHive;
 clone->temperaturePlate = runningProgram->temperaturePlate;
 runningProgram = clone;

 logger.info(F("extending program %s by %dmin"), runningProgram->name, duration);
 startTime = millis();
 status.setSystemState(Status::ready);
 status.setSystemState(Status::running);
 eventHandler->sendEvent(startProgram);
 }
 */

void Hive::handleProgramChange(Program program) {
	logger.debug(F("hive noticed program change"));
	if (program.running) {
		logger.debug(F("closing heater relay"));
		digitalWrite(configuration.getIO()->heaterRelay, HIGH);
	} else {
		logger.debug(F("scheduling release of heater relay"));
		//TODO set flag to open relay in a later cycle to first allow all heaters to go to 0 power
	}
}
