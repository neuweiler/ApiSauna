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

    //TODO move to EEPROM
    plateConfigs.push_back(PlateConfig(1, CFG_ADDR_TEMP_SENSOR_1, CFG_IO_HEATER_1, CFG_IO_FAN_1));
    plateConfigs.push_back(PlateConfig(2, CFG_ADDR_TEMP_SENSOR_2, CFG_IO_HEATER_2, CFG_IO_FAN_2));
    plateConfigs.push_back(PlateConfig(3, CFG_ADDR_TEMP_SENSOR_3, CFG_IO_HEATER_3, CFG_IO_FAN_3));
    plateConfigs.push_back(PlateConfig(4, CFG_ADDR_TEMP_SENSOR_4, CFG_IO_HEATER_4, CFG_IO_FAN_4));

    program.hiveTemperature = 420; // the target temperature (in 0.1 deg C)
    program.deratingHiveTemperature = 300;
    program.plateTemperature = 730; // the maximum temperature of the heater plates (in 0.1 deg C)
    program.deratingPlateTemperature = 580;
    program.preHeatDuration = 1800; // the duration of the pre-heating cycle (in sec)
    program.duration = 14400; // how long the program should run (in sec)
    program.fanSpeed = 127; // the maximum fan speed (0-255)
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

ControllerProgram* Controller::getProgram()
{
    return &program;
}

uint16_t Controller::getTimeRunning()
{
    return (millis() - program.startTime) / 1000;
}

uint16_t Controller::getTimeRemaining()
{
    return program.duration - getTimeRunning();
}

int16_t Controller::getMaxHiveTemperature() {
    return hiveTemperature;
}

void Controller::startProgram(ControllerProgram controllerProgram)
{
    this->program = controllerProgram;
}

void Controller::stopProgram()
{
    // TODO !!
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
void Controller::assignTemperatureSensors(SimpleList<SensorAddress> addressList)
{
    uint8_t plateNumber = 1;
    for (SimpleList<SensorAddress>::iterator itrAddress = addressList.begin(); itrAddress != addressList.end(); ++itrAddress) {
        bool foundInConfig = false;
        for (SimpleList<PlateConfig>::iterator itrConfig = plateConfigs.begin(); itrConfig != plateConfigs.end(); ++itrConfig) {
            if (itrAddress->value == itrConfig->sensorAddress.value) {
                foundInConfig = true;
                Logger::info("attaching sensor %#lx%lx, heater pin %d, fan pin %d to plate #%d", itrConfig->sensorAddress.high,
                        itrConfig->sensorAddress.low, itrConfig->heaterPin, itrConfig->fanPin, plateNumber++);
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
    assignTemperatureSensors(addressList);
    if (plateConfigs.size() != plates.size()) {
        Logger::error("unable to locate all temperature sensors of all configured plates (%d of %d) !!", plates.size(), plateConfigs.size());
        status.setSystemState(Status::error);
    }
    //TODO set humidifier fan, evap and sensor pins


}

/**
 * This is the main control loop.
 */
void Controller::loop()
{
    hiveTemperature = -999;
    //TODO get max hive temperature and adjust plate's max temperature accordingly (incl. derating)
    for (SimpleList<TemperatureSensor>::iterator itr = hiveTempSensors.begin(); itr != hiveTempSensors.end(); ++itr) {
        itr->retrieveData();
        hiveTemperature = max(hiveTemperature, itr->getTemperatureCelsius());
    }

    int16_t maxPlateTemperature = 0;
    if (hiveTemperature != -999 && hiveTemperature < program.hiveTemperature) {
        if (hiveTemperature < program.deratingHiveTemperature) {
            maxPlateTemperature = program.plateTemperature;
        } else {
            maxPlateTemperature = program.plateTemperature * (program.hiveTemperature - hiveTemperature) / (program.hiveTemperature - program.deratingHiveTemperature); // derating
        }
//        Logger::debug("max plate temperature: %d", maxPlateTemperature / 10);
    }

    switch (status.getSystemState()) {
    case Status::init:
        Logger::error("controller loop must not be called while initialising!");
        break;
    case Status::ready:
        break;
    case Status::preHeat:
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            itr->setDeratingTemperature(program.deratingPlateTemperature); // TODO move to init
            itr->setFanSpeed(program.fanSpeed); // TODO move to init
            itr->setMaximumTemperature(maxPlateTemperature);
            itr->loop();
        }
        humidifier.loop();
        break;
    case Status::running:
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            itr->setMaximumTemperature(maxPlateTemperature);
            itr->loop();
        }
        humidifier.loop();
        break;
    case Status::shutdown:
    case Status::error:

        break;
    }

    TemperatureSensor::prepareData();
    delay(CFG_LOOP_DELAY);
}
