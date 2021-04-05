/*
 * HiveFactory.cpp
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

#include "HiveFactory.h"

HiveFactory::HiveFactory() {
}

HiveFactory::~HiveFactory() {
}

void HiveFactory::create() {
	SimpleList<SensorAddress> addressList = TemperatureSensor::detectTemperatureSensors();

	SimpleList<Plate *> plates = createPlates(addressList);
	SimpleList<TemperatureSensor *> hiveSensors = createHiveSensors(addressList);
	SimpleList<ThermalZone *> zones = createThermalZones(hiveSensors, plates);

	if (zones.size() == 0) {
		eventHandler.publish(EventListener::ERROR, F("no zones defined"));
	}

	for (SimpleList<Plate *>::iterator plate = plates.begin(); plate != plates.end(); ++plate) {
		(*plate)->initialize();
	}
	for (SimpleList<ThermalZone *>::iterator thermalZone = zones.begin(); thermalZone != zones.end(); ++thermalZone) {
		(*thermalZone)->initialize();
	}
}

/**
 * Assign temperature sensors to heating plates according to configuration.
 */
SimpleList<Plate *> HiveFactory::createPlates(SimpleList<SensorAddress> &addressList) {
	ConfigurationIO *configIO = configuration.getIO();
	ConfigurationSensor *configSensor = configuration.getSensor();
	SimpleList<Plate *> plates;

	logger.info(F("creating plates and assigning sensor, fan and heater to each"));

	plates.reserve(configuration.getParams()->numberOfPlates);
	for (int i = 0; i < configuration.getParams()->numberOfPlates; i++) {
		if (configSensor->addressPlate[i].value != 0 && findSensor(addressList, configSensor->addressPlate[i])
				&& configIO->fan[i] != 0 && configIO->heater[i] != 0) {
			logger.info(F("  plate #%d gets sensor %#08lx%08lx, heater pin %d, fan pin %d"), i + 1,
					configSensor->addressPlate[i].high, configSensor->addressPlate[i].low, configIO->heater[i],
					configIO->fan[i]);
			plates.push_back(new Plate(i));
		}
	}

	if (configuration.getParams()->numberOfPlates != plates.size()) {
		logger.error(F("unable to locate all configured plate sensors (%d of %d) !!"), plates.size(),
				configuration.getParams()->numberOfPlates);
		eventHandler.publish(EventListener::ERROR, F("Not all plate sensors found"));
	}
	return plates;
}

/**
 * Verify that all configured hive temperature sensors are found in the search for sensors.
 */
SimpleList<TemperatureSensor *> HiveFactory::createHiveSensors(SimpleList<SensorAddress> &addressList) {
	ConfigurationSensor *configSensor = configuration.getSensor();
	SimpleList<TemperatureSensor *> hiveSensors;

	logger.info(F("assigning hive sensors"));

	for (int i = 0; configSensor->addressHive[i].value != 0 && i < CFG_MAX_NUMBER_PLATES; i++) {
		if (findSensor(addressList, configSensor->addressHive[i])) {
			logger.info(F("  sensor %#08lx%08lx is hive sensor #%d"), configSensor->addressHive[i].high,
					configSensor->addressHive[i].low, i + 1);
			hiveSensors.push_back(new TemperatureSensor(configSensor->addressHive[i], i));
		} else {
			logger.error(F("unable to locate all configured hive sensors (%#l08x%08lx missing) !!"),
					configSensor->addressHive[i].high, configSensor->addressHive[i].low);
			eventHandler.publish(EventListener::ERROR, F("Not all hive sensors found"));
		}
	}
	return hiveSensors;
}

bool HiveFactory::findSensor(SimpleList<SensorAddress> &addressList, SensorAddress address) {
	for (SimpleList<SensorAddress>::iterator entry = addressList.begin(); entry != addressList.end(); ++entry) {
		if (entry->value == address.value)
			return true;
	}
	return false;
}

SimpleList<ThermalZone *> HiveFactory::createThermalZones(SimpleList<TemperatureSensor *> sensors,
		SimpleList<Plate *> plates) {
	uint8_t sizeSensors = sensors.size();
	uint8_t sizePlates = plates.size();
	SimpleList<ThermalZone *> zones;

	logger.info(F("creating thermal zones with %d hive sensors and %d plates"), sizeSensors, sizePlates);

	if (configuration.getParams()->thermalZones == 0) {
		if (sizeSensors == sizePlates) { // 1:1 mapping of sensors and plates
			logger.info(F("  assigning 1 plate and 1 hive sensor per zone"));

			SimpleList<TemperatureSensor *>::iterator sensor = sensors.begin();
			for (SimpleList<Plate *>::iterator plate = plates.begin(); plate != plates.end(); ++plate) {
				ThermalZone *zone = new ThermalZone();
				zone->addPlate(*plate);
				zone->addSensor(*sensor++);
				zones.push_back(zone);
			}
		} else if (sizeSensors == sizePlates + 1) { // map two sensors to each plate
			logger.info(F("  assigning 2 hive sensors per 1 plate per zone"));

			SimpleList<TemperatureSensor *>::iterator sensor = sensors.begin();
			for (SimpleList<Plate *>::iterator plate = plates.begin(); plate != plates.end(); ++plate) {
				ThermalZone *zone = new ThermalZone();
				zone->addPlate(*plate);
				zone->addSensor(*sensor);
				zone->addSensor(*(sensor++ + 1));
				zones.push_back(zone);
			}
		} else if (sizeSensors + 1 == sizePlates) { // map two plates to each sensor
			logger.info(F("  assigning 2 plates per 1 hive sensor per zone"));

			SimpleList<Plate *>::iterator plate = plates.begin();
			for (SimpleList<TemperatureSensor *>::iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {
				ThermalZone *zone = new ThermalZone();
				zone->addSensor(*sensor);
				zone->addPlate(*plate);
				zone->addPlate(*(plate++ + 1));
				zones.push_back(zone);
			}
		}
		logger.info(F("  created %d thermal zones"), zones.size());
	}
	if (zones.size() == 0) { // combine all sensors and plates to one zone
		logger.info(F("  assigning all plates and hive sensors to one zone"));

		ThermalZone *zone = new ThermalZone();
		for (SimpleList<TemperatureSensor *>::iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {
			zone->addSensor(*sensor);
		}
		for (SimpleList<Plate *>::iterator plate = plates.begin(); plate != plates.end(); ++plate) {
			zone->addPlate(*plate);
		}
		zones.push_back(zone);
	}
	return zones;
}
