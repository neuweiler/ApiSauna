/*
 * Configuration.h
 *
 *  Although the AT Mega has a 4k EEPROM, we limit ourselves to 1k to remain compatible with other models
 *
 *  The 1k is split into 4 blocks of 255+1 bytes so the can be individually expanded without impacting other blocks
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

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include "config.h"
#include "Logger.h"
#include "Crc.h"
#include <EEPROM.h>
#include "EventHandler.h"

#define CONFIG_ADDRESS_PARAMS       0
#define CONFIG_ADDRESS_IO           256
#define CONFIG_ADDRESS_SENSOR       512
//#define CONFIG_ADDRESS_STATISTICS   768
#define CFG_EEPROM_CONFIG_TOKEN     0xbee

typedef union {
	uint64_t value;
	struct {
		uint32_t low;
		uint32_t high;
	};
	struct {
		uint16_t s0;
		uint16_t s1;
		uint16_t s2;
		uint16_t s3;
	};
	uint8_t byte[8];
} SensorAddress;

class ConfigurationParams {
public:
	uint32_t crc;
	uint16_t token; // identify if an apiSauna config is stored in the eeprom
	uint8_t version; // version counter (of configuration, not software)

	uint8_t numberOfPlates; // the number of heater plates / fans installed (default: 4)
	uint8_t maxHeaterPower; // the maximum heater power to be applied by PWM on a single plate (0-255, default: 170)
	uint8_t minFanSpeed; // the minimum setting of the heater fan speed (default: 10)
	uint16_t hiveOverTemp; // the temperature at which an emergency is declared because the hive overheats (in 0.1°C, default: 460)
	uint16_t hiveMaxTemp; // the temperature at which all heaters are paused (in 0.1°C, default: 350)
	uint16_t plateOverTemp; // the temperature at which plates are overheated an the program aborts (in 0.1°C, default: 850)
	uint8_t usePWM; // should we use PWM or simple on/off at every loop for plates (0 or 1, default: 0)
	uint8_t maxConcurrentHeaters; // the number of allows heaters active at the same time when not using PWM (default: 2)
	uint8_t humidifierFanDryTime; // time to keep humidifier fan running after stopping the vaporizer to allow it to dry (in min, default: 2)
	uint8_t loglevel; // the loglevel
	uint8_t thermalZones; // in what way the thermal zones should be aggregated with heatern plates
	// 21 bytes used
};

class ConfigurationIO {
public:
	uint32_t crc; // CRC of this config block

	uint8_t heartbeat; // the pin the heartbeat led is attached to (default: 13)
	uint8_t temperatureSensor; //  pin to which the data line of the single wire temperature sensors is connected (default: 4)
	uint8_t humiditySensor; // pin to which the data line of the humidity sensor is connected to (default: 9)
	uint8_t humiditySensorType; // type of the used humidity sensor (11, 21, 22, default: 22)
	uint8_t vaporizer; // pin which controls the vaporizer (default: 5)
	uint8_t humidifierFan; // pin which controls the humidifier fan (default: 6)
	uint8_t heaterRelay; // pin which controls the main relay for all heaters, emergency switch (default: 54 = A0)
	uint8_t heater[CFG_MAX_NUMBER_PLATES]; // the pin assignments to control the heaters (default: 11, 12, 2, 3)
	uint8_t fan[CFG_MAX_NUMBER_PLATES]; // the pin assignments to control the heater fans (default: 7, 8, 44, 45)
	uint8_t buttonNext; // pin to which the next button is connected (default: 55 = A1)
	uint8_t buttonSelect; // pin to which the select button is connected (default: 56 = A2)
	uint8_t beeper; // pin which is connected to the piezo beeper (default: 10)
	uint8_t lcdRs; // pin which controls the lcd's RS pin (default: 22)
	uint8_t lcdEnable; // pin which controls the lcd's enable pin (default: 23)
	uint8_t lcdD4; // pin which controls the lcd's D0 pin (default: 24)
	uint8_t lcdD5; // pin which controls the lcd's D1 pin (default: 25)
	uint8_t lcdD6; // pin which controls the lcd's D2 pin (default: 26)
	uint8_t lcdD7; // pin which controls the lcd's D3 pin (default: 27)
	// 22 bytes used
};

class ConfigurationSensor {
public:
	uint32_t crc;

	SensorAddress addressPlate[CFG_MAX_NUMBER_PLATES]; // the addresses of the temperature sensors assigned to the heater plates (0=disabled)
	SensorAddress addressHive[CFG_MAX_NUMBER_PLATES]; // the addresses of the temperature sensors assigned to the hive (0=disabled)
	// 246 bytes used
};

class Configuration {
public:
	Configuration();
	virtual ~Configuration();
	ConfigurationIO* getIO();
	ConfigurationParams* getParams();
	ConfigurationSensor* getSensor();
	bool load();
	void save();
	void reset();

private:
	Configuration(Configuration const&); // copy disabled
	void operator=(Configuration const&); // assigment disabled
	void updateCrc();
};

extern Configuration configuration;

#endif /* CONFIGURATION_H_ */
