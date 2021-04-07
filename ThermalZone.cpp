/*
 * ThermalZone.cpp
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

#include "ThermalZone.h"

uint8_t ThermalZone::zoneCounter = 0;

ThermalZone::ThermalZone() {
	pid = NULL;
	actualTemperature = -999;
	targetTemperature = 0;
	plateTemperature = 0;
	plateMaxTemperatureProgram = 0;
	plateTargetTemperature = 0;
	temperatureHigh = false;
	requestData = true;

	status.id = zoneCounter++;
	status.temperatureTarget = 0;
	status.temperatureActual = 0;
	preHeat = false;
}

ThermalZone::~ThermalZone() {
	logger.debug(F("ThermalZone %d destroyed"), status.id);
	if (pid) {
		delete pid;
		pid = NULL;
	}
}

void ThermalZone::handleEvent(Event event, va_list args) {
	switch (event) {
	case PROCESS_INPUT:
		if (requestData) {
			TemperatureSensor::prepareData();
			requestData = false;
		}
		break;
	case PROCESS:
		process();
		requestData = true;
		break;
	case PROGRAM_PREHEAT:
		preHeat = true;
		programChange(va_arg(args, Program));
		break;
	case PROGRAM_RUN:
		preHeat = false;
		programChange(va_arg(args, Program));
		break;
	case PROGRAM_UPDATE:
		programChange(va_arg(args, Program));
		break;
	case PROGRAM_STOP:
	case TEMPERATURE_ALERT:
		break;
	case PROGRAM_PAUSE:
		break;
	case PROGRAM_RESUME:
		break;
	}
}

void ThermalZone::initialize() {
	logger.info(F("initializing thermal zone %d (%x)"), status.id, this);

	initPid();
	eventHandler.subscribe(this);
}

void ThermalZone::process() {
	actualTemperature = retrieveTemperature();
	status.temperatureActual = actualTemperature;

	int16_t plateTemp = calculatePlateTargetTemperature();
	for (SimpleList<Plate *>::iterator plate = plates.begin(); plate != plates.end(); ++plate) {
		(*plate)->setTargetTemperature(plateTemp);
	}

	if (actualTemperature > configuration.getParams()->hiveMaxTemp && !temperatureHigh) {
		eventHandler.publish(TEMPERATURE_HIGH); // pause all heaters, activate fresh air vent
		temperatureHigh = true;
	}
	if (actualTemperature > configuration.getParams()->hiveOverTemp) {
		eventHandler.publish(TEMPERATURE_ALERT); // abort program?, alert !!
	}
	if (actualTemperature < configuration.getParams()->hiveMaxTemp && temperatureHigh) {
		eventHandler.publish(TEMPERATURE_NORMAL);
		temperatureHigh = false;
	}

	eventHandler.publish(EventListener::STATUS_ZONE, status);
}

void ThermalZone::programChange(const Program &program) {
	logger.info(F("thermal zone noticed program change"));

	// adjust the PID which defines the target temperature of the plates based on the hive temp
	pid->SetOutputLimits((preHeat ? program.temperaturePreHeat : program.temperatureHive), program.temperaturePlate);
	pid->SetTunings(program.hiveKp, program.hiveKi, program.hiveKd);
	plateTargetTemperature = program.temperaturePlate;
	plateMaxTemperatureProgram = program.temperaturePlate;

	targetTemperature = (preHeat ? program.temperaturePreHeat : program.temperatureHive);
	status.temperatureTarget = targetTemperature;
}

/**
 * Initialize the PID to define the plateTemperature based on the targetTemperature and the
 * actual temperature of the zone.
 */
void ThermalZone::initPid() {
	pid = new PID(&actualTemperature, &plateTemperature, &targetTemperature, 0, 0, 0, DIRECT);
	pid->SetOutputLimits(0, configuration.getParams()->plateOverTemp);
	pid->SetSampleTime(CFG_LOOP_DELAY);
	pid->SetMode(AUTOMATIC);
}

/**
 * Retrieve temperature data from all assigned sensors and return highest value (in 0.1 deg C)
 */
int16_t ThermalZone::retrieveTemperature() {
	int16_t max = -999;

	for (SimpleList<TemperatureSensor *>::iterator sensor = temperatureSensors.begin();
			sensor != temperatureSensors.end(); ++sensor) {
		(*sensor)->retrieveData();
		max = max(max, (*sensor)->getTemperatureCelsius());
	}
	return max;
}

/**
 * Calculate the desired plate temperature based on the hive temperature and the current state. (in 0.1 deg C)
 */
int16_t ThermalZone::calculatePlateTargetTemperature() {
	if (actualTemperature == -999) {
		return 0;
	}

	pid->Compute();

	// don't set directly as plateTemperature tends to jump. Dampen with 0.1 deg per second
	if (plateTemperature > plateTargetTemperature)
		plateTargetTemperature++;
	else if (plateTemperature < plateTargetTemperature)
		plateTargetTemperature--;

	plateTargetTemperature = constrain(plateTargetTemperature, 0, plateMaxTemperatureProgram);

	if (logger.isDebug()) {
		logger.debug(F("zone actual temperature: %d, plate target temperature: %d"), actualTemperature, plateTargetTemperature);
	}

	return plateTargetTemperature;
}

void ThermalZone::addSensor(TemperatureSensor *sensor) {
	temperatureSensors.push_back(sensor);
}

void ThermalZone::addPlate(Plate *plate) {
	plates.push_back(plate);
}
