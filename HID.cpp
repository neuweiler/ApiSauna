/*
 * HID.cpp
 *
 * Controls the output to a display (LCD screen) and reacts on human input
 * to control the sauna.
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

#include "HID.h"

HID::HID()
{
    controller = NULL;
}

HID::~HID()
{
    // TODO Auto-generated destructor stub
}

void HID::init(Controller* controller)
{
    this->controller = controller;
}

void HID::loop()
{
    if (controller != NULL) {
        Logger::debug("time running: %d:%d, time remaining: %d:%d", controller->getTimeRunning() / 60, controller->getTimeRunning() % 60,
                controller->getTimeRemaining() / 60, controller->getTimeRemaining() % 60);

        SimpleList<TemperatureSensor> hiveSensors = controller->getHiveTempSensors();
        int sensorNumber = 1;
        for (SimpleList<TemperatureSensor>::iterator itr = hiveSensors.begin(); itr != hiveSensors.end(); ++itr) {
            int16_t temp = itr->getTemperatureCelsius();
            Logger::debug("hive temp: temp=%d.%d C", sensorNumber++, temp / 10, temp % 10);
        }

        SimpleList<Plate> plates = controller->getPlates();
        int plateNumber = 1;
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            int16_t temp = itr->getTemperature();
            Logger::debug("plate %d: temp=%d.%d C, power=%d, speed=%d", plateNumber++, temp / 10, temp % 10, itr->getPower(), itr->getFanSpeed());
        }

        Humidifier humidifier = controller->getHumidifier();
        Logger::debug("humidity: relHumidity=%d, enabled=%d, speed=%d", humidifier.getHumidity(), humidifier.getEvaporatorMode(),
                humidifier.getFanSpeed());
    }

}
