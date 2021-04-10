/*
 * ThermalZone.h
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

#ifndef THERMALZONE_H_
#define THERMALZONE_H_

#include "SimpleList.h"
#include "Configuration.h"
#include "TemperatureSensor.h"
#include "Plate.h"
#include "PID_v1.h"

#include "EventHandler.h"
#include "Program.h"

class ThermalZone: public EventListener {
public:
	ThermalZone();
	virtual ~ThermalZone();
	void initialize();
	void handleEvent(Event event, va_list args);
	void addSensor(TemperatureSensor *sensor);
	void addPlate(Plate *plate);

private:
	void process();
	void programChange(const Program *program);
	void initPid();
	int16_t retrieveTemperature();
	int16_t calculatePlateTargetTemperature();
	uint8_t calculateFanLevel();

	static uint8_t zoneCounter;
	static bool requestData;
	SimpleList<TemperatureSensor *> temperatureSensors;
	SimpleList<Plate *> plates;
	PID *pid; // pointer to PID controller
	double actualTemperature, targetTemperature, plateTemperature; // values for/set by the PID controller
	int16_t plateTargetTemperature;
	int16_t plateMaxTemperatureProgram;
	bool temperatureHigh;
	StatusZone status;
	bool preHeat;
};

#endif /* THERMALZONE_H_ */
