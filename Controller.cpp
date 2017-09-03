/*
 * Controller.cpp
 *
 * The main controller which coordinates and controlls all parts of the
 * bee sauna..
 *
 Copyright (c) 2017 Michael Neuweiler

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

#include "Controller.h"

Controller::Controller()
{
    hiveTemperature = -999;
    statusLed = false;
    runningProgram = NULL;
    offset = 0;

    //TODO move to EEPROM
    plateConfigs.push_back(PlateConfig(1, CFG_ADDR_TEMP_SENSOR_1, CFG_IO_HEATER_1, CFG_IO_FAN_1));
    plateConfigs.push_back(PlateConfig(2, CFG_ADDR_TEMP_SENSOR_2, CFG_IO_HEATER_2, CFG_IO_FAN_2));
    plateConfigs.push_back(PlateConfig(3, CFG_ADDR_TEMP_SENSOR_3, CFG_IO_HEATER_3, CFG_IO_FAN_3));
    plateConfigs.push_back(PlateConfig(4, CFG_ADDR_TEMP_SENSOR_4, CFG_IO_HEATER_4, CFG_IO_FAN_4));

    ControllerProgram program;
    program.preHeatTemperature = 380; // 38 deg C
    program.preHeatFanSpeed = 80;
    program.preHeatDuration = 1800; // 30min
    program.hiveTemperature = 420; // 42 deg C
    program.deratingHiveDelta = 20; // derating begins 2 deg C below target temp
    program.plateTemperature = 730;
    program.deratingPlateDelta = 50; // derating begins 15 deg C below target temp
    program.fanSpeed = 10; // minimum
    program.humidityMinimum = 35;
    program.humidityMaximum = 60;
    program.fanSpeedHumidifier = 240; // only works from 230 to 255
    program.duration = 14400; // 4 hours
    programs.push_back(program);
}

Controller::~Controller()
{
    // TODO Auto-generated destructor stub
}

SimpleList<TemperatureSensor>* Controller::getHiveTempSensors()
{
    return &hiveTempSensors;
}

SimpleList<Plate>* Controller::getPlates()
{
    return &plates;
}

Humidifier* Controller::getHumidifier()
{
    return &humidifier;
}

ControllerProgram* Controller::getRunningProgram()
{
    return runningProgram;
}

SimpleList<ControllerProgram>* Controller::getPrograms()
{
    return &programs;
}

uint16_t Controller::getTimeRunning()
{
    if (runningProgram != NULL && (status.getSystemState() == Status::preHeat || status.getSystemState() == Status::running)) {
        return (millis() - runningProgram->startTime) / 1000;
    }
    return 0;
}

uint16_t Controller::getTimeRemaining()
{
    if (runningProgram != NULL) {
        if (status.getSystemState() == Status::preHeat) {
            return runningProgram->preHeatDuration - getTimeRunning();
        }
        if (status.getSystemState() == Status::running) {
            return runningProgram->duration - getTimeRunning();
        }
    }
    return 0;
}

int16_t Controller::getHiveTemperature()
{
    return hiveTemperature; // updated in the loop()
}

int16_t Controller::getHiveTargetTemperature()
{
    if (runningProgram != NULL) {
        if (status.getSystemState() == Status::preHeat) {
            return runningProgram->preHeatTemperature;
        }
        if (status.getSystemState() == Status::running) {
            return runningProgram->hiveTemperature;
        }
    }
    return 0;
}

void Controller::startProgram(ControllerProgram *controllerProgram)
{
    if (controllerProgram != NULL && status.setSystemState(Status::preHeat) == Status::preHeat) {
        runningProgram = controllerProgram;

        // init all plate parameters
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            itr->setDeratingDelta(runningProgram->deratingPlateDelta);
            itr->setFanSpeed(runningProgram->preHeatFanSpeed);
            itr->setMaximumPower(CFG_MAX_HEATER_POWER);
        }
        humidifier.setFanSpeed(runningProgram->fanSpeedHumidifier);
        humidifier.setMinHumidity(runningProgram->humidityMinimum);
        humidifier.setMaxHumidity(runningProgram->humidityMaximum);

        digitalWrite(CFG_IO_HEATER_MAIN_SWITCH, HIGH);
        delay(500); // give it some time to close
    }
}

void Controller::stopProgram()
{
    runningProgram = NULL;
    status.setSystemState(Status::shutdown);
}

/**
 *  Find all temperature sensor addresses (DS18B20).
 */
SimpleList<SensorAddress> Controller::findTemperatureSensors()
{
    SimpleList<SensorAddress> addressList;

    TemperatureSensor::resetSearch();
    Logger::info("searching temperature sensors...");

    while (true) {
        SensorAddress address = TemperatureSensor::search();
        if (address.value == 0)
            break;
        Logger::debug("  found sensor: addr=%#lx%lx", address.high, address.low);
        addressList.push_back(address);
    }
    TemperatureSensor::prepareData(); // kick the sensors to prepare data
    return addressList;
}

/**
 * Assign all found temperature sensors either to heating plates or use them to measure the
 * hive's temperature.
 */
void Controller::assignTemperatureSensors(SimpleList<SensorAddress> *addressList)
{
    for (SimpleList<SensorAddress>::iterator itrAddress = addressList->begin(); itrAddress != addressList->end(); ++itrAddress) {
        bool foundInConfig = false;
        for (SimpleList<PlateConfig>::iterator itrConfig = plateConfigs.begin(); itrConfig != plateConfigs.end(); ++itrConfig) {
            if (itrAddress->value == itrConfig->sensorAddress.value) {
                foundInConfig = true;
                Logger::info("attaching sensor %#lx%lx, heater pin %d, fan pin %d to plate #%d", itrConfig->sensorAddress.high,
                        itrConfig->sensorAddress.low, itrConfig->heaterPin, itrConfig->fanPin, itrConfig->id);
                plates.push_back(Plate(itrConfig->id, itrConfig->sensorAddress, itrConfig->heaterPin, itrConfig->fanPin));
            }
        }
        if (!foundInConfig) {
            Logger::info("using temp sensor %#lx%lx to measure hive temperature", itrAddress->high, itrAddress->low);
            hiveTempSensors.push_back(TemperatureSensor(*itrAddress));
        }
    }
}

/**
 * Initialize the controller and referenced devices.
 */
void Controller::init()
{
    SimpleList<SensorAddress> addressList = findTemperatureSensors();
    assignTemperatureSensors(&addressList);
    if (plateConfigs.size() != plates.size()) {
        Logger::error("unable to locate all temperature sensors of all configured plates (%d of %d) !!", plates.size(), plateConfigs.size());
        status.setSystemState(Status::error);
    }
    humidifier.setVaporizerPin(CFG_IO_VAPORIZER);
    humidifier.setFanPin(CFG_IO_FAN_HUMIDIFIER);
    humidifier.setSensorPin(CFG_IO_HUMIDITY_SENSOR);
}

// turn
void Controller::powerDownDevices()
{
    analogWrite(CFG_IO_HEATER_1, 0);
    analogWrite(CFG_IO_HEATER_2, 0);
    analogWrite(CFG_IO_HEATER_3, 0);
    analogWrite(CFG_IO_HEATER_4, 0);
    analogWrite(CFG_IO_FAN_1, CFG_MIN_FAN_SPEED);
    analogWrite(CFG_IO_FAN_2, CFG_MIN_FAN_SPEED);
    analogWrite(CFG_IO_FAN_3, CFG_MIN_FAN_SPEED);
    analogWrite(CFG_IO_FAN_4, CFG_MIN_FAN_SPEED);
    analogWrite(CFG_IO_VAPORIZER, 0);
    analogWrite(CFG_IO_FAN_HUMIDIFIER, 0);
    delay(10); // wait a bit before trying to open the relay
    digitalWrite(CFG_IO_HEATER_MAIN_SWITCH, LOW);
}

/**
 * Retrieve temperature data from all sensors and return highest value (in 0.1 deg C)
 */
int16_t Controller::retrieveHiveTemperatures()
{
    int16_t max = -999;

    for (SimpleList<TemperatureSensor>::iterator itr = hiveTempSensors.begin(); itr != hiveTempSensors.end(); ++itr) {
        itr->retrieveData();
        max = max(max, itr->getTemperatureCelsius());
    }
    return max;
}

/**
 * Calculate the maximum plate temperature based on the hive temperature and the current state. (in 0.1 deg C)
 */
int16_t Controller::calculateMaxPlateTemperature()
{
    int16_t hiveTarget, deratingTemp;

    if (hiveTemperature == -999 || runningProgram == NULL) {
        return 0;
    }

    if (status.getSystemState() == Status::preHeat) {
        hiveTarget = runningProgram->preHeatTemperature;
        deratingTemp = runningProgram->preHeatTemperature - runningProgram->deratingHiveDelta;
    } else {
        hiveTarget = runningProgram->hiveTemperature;
        deratingTemp = runningProgram->hiveTemperature - runningProgram->deratingHiveDelta;
    }

    if (hiveTemperature < deratingTemp) {
        return runningProgram->plateTemperature; // below derating temp --> full power
    } else {
        if (hiveTarget == deratingTemp) {
            return hiveTarget; // no derating but above hive target --> reduce plate temp to hive target temp
        } else {
            return map(hiveTemperature, hiveTarget, deratingTemp, hiveTarget + offset, runningProgram->plateTemperature) + 10; // derating
        }
    }
}

/**
 * Check if a state has reached its execution time and switch to next state.
 */
void Controller::updateProgramState()
{
    if (hiveTemperature > CFG_HIVE_OVER_TEMPERATURE) {
        Logger::error("!!!!!!!!!!!!!!!!! ALERT !!!!!!!!!!!!!!!!!!!! OVER-TEMPERATURE IN HIVE DETECTED !!!!!!!!!!!!!!! Trying to recover...");
        status.setSystemState(Status::overtemp);
    }
    if (status.getSystemState() == Status::overtemp && hiveTemperature < CFG_HIVE_TEMPERATURE_RECOVER) {
        Logger::info("Recovered from over-temperature in hive, shutting-down as precaution");
        status.setSystemState(Status::shutdown);
    }

    // switch to running if pre-heating cycle is finished
    if (status.getSystemState() == Status::preHeat && getTimeRemaining() <= 0 && runningProgram) {
        status.setSystemState(Status::running);
        runningProgram->startTime = millis();
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            itr->setFanSpeed(runningProgram->fanSpeed);
        }
    }

    // switch to shutdown when program is finished
    if (status.getSystemState() == Status::running && getTimeRemaining() <= 0) {
        status.setSystemState(Status::shutdown);
    }
}

/**
 * This is the main control loop.
 */
void Controller::loop()
{
    hiveTemperature = retrieveHiveTemperatures();
    updateProgramState();

    switch (status.getSystemState()) {
    case Status::init:
        Logger::error("controller loop must not be called while initialising!");
        break;
    case Status::ready:
        break;
    case Status::preHeat:
    case Status::running: {
        int16_t plateTemp = calculateMaxPlateTemperature();
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            itr->setMaximumTemperature(plateTemp);
            itr->loop();
        }
        humidifier.loop();
        break;
    }
    case Status::overtemp: // shut-down heaters, full blow, full vaporizer (?) + fresh air !!
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            itr->setMaximumPower(0);
            itr->setFanSpeed(CFG_MIN_FAN_SPEED);
            itr->loop();
        }
        humidifier.setMinHumidity(99);
        humidifier.setMaxHumidity(100);
        humidifier.setFanSpeed(255);
        humidifier.loop();
        delay(10); // wait a bit before trying to open the relay
        digitalWrite(CFG_IO_HEATER_MAIN_SWITCH, LOW);
        break;
    case Status::shutdown:
    case Status::error:
        powerDownDevices();
        break;
    }

    beeper.loop();
    TemperatureSensor::prepareData();

    statusLed = !statusLed;
    digitalWrite(CFG_IO_BLINK_LED, statusLed); // some kind of heart-beat
}
