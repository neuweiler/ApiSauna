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
    //TODO move to EEPROM
    plateConfigs.push_back(PlateConfig(0xf70315a86f2fff28, 3, 7));
    plateConfigs.push_back(PlateConfig(0xf80315a86f2fff28, 4, 8));
    plateConfigs.push_back(PlateConfig(0xf90315a86f2fff28, 5, 9));
    plateConfigs.push_back(PlateConfig(0xfa0315a86f2fff28, 6, 10));
}

Controller::~Controller()
{
    // TODO Auto-generated destructor stub
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
        Logger::info("  found sensor: addr=%#lx%lx", address.high, address.low);
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
                Logger::debug("attaching sensor %#lx%lx, heater pin %d, fan pin %d to plate #%d", itrConfig->sensorAddress.high,
                        itrConfig->sensorAddress.low, itrConfig->heaterPin, itrConfig->fanPin, plateNumber++);
                plates.push_back(Plate(itrConfig->sensorAddress, itrConfig->heaterPin, itrConfig->fanPin));
            }
        }
        if (!foundInConfig) {
            Logger::error("using temp sensor %#lx%lx to measure hive temperature", itrAddress->high, itrAddress->low);
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
    }
}

/**
 * This is the main control loop.
 */
void Controller::loop()
{
    int plateNumber = 1;
    for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
        itr->loop();

        if (Logger::isDebug()) {
            int16_t temp = itr->getTemperature();
            Logger::debug("plate %d: temp=%d.%d C, power=%d", plateNumber, temp / 10, temp % 10, itr->getHeaterPower());
        }
        plateNumber++;
    }

    TemperatureSensor::prepareData();
    delay(CFG_LOOP_DELAY);
}

