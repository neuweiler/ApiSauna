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
}

HID::~HID()
{
}

void HID::init()
{
}

void HID::loop()
{
    Logger::debug("time running: %s, time remaining: %s, status: %s", convertTime(controller.getTimeRunning()).c_str(),
            convertTime(controller.getTimeRemaining()).c_str(), status.systemStateToStr(status.getSystemState()).c_str());

    SimpleList<TemperatureSensor> *hiveSensors = controller.getHiveTempSensors();
    for (SimpleList<TemperatureSensor>::iterator itr = hiveSensors->begin(); itr != hiveSensors->end(); ++itr) {
        int16_t temp = itr->getTemperatureCelsius();
        Logger::debug("sensor %#lx%lx: %s C", itr->getAddress().high, itr->getAddress().low, toDecimal(temp, 10).c_str());
    }
    Logger::debug("hive temp: %s C, target: %s C", toDecimal(controller.getHiveTemperature(), 10).c_str(),
            toDecimal(controller.getHiveTargetTemperature(), 10).c_str());

    SimpleList<Plate> *plates = controller.getPlates();
    for (SimpleList<Plate>::iterator itr = plates->begin(); itr != plates->end(); ++itr) {
        Logger::debug("plate %d: temp=%s C, target=%s C, power=%d (max=%d), speed=%d", itr->getId(), toDecimal(itr->getTemperature(), 10).c_str(),
                toDecimal(itr->getMaximumTemperature(), 10).c_str(), itr->getPower(), itr->getMaximumPower(), itr->getFanSpeed());
    }

    Humidifier *humidifier = controller.getHumidifier();
    Logger::debug("humidity: relHumidity=%d (%d-%d), enabled=%d, speed=%d", humidifier->getHumidity(), humidifier->getMinHumidity(),
            humidifier->getMaxHumidity(), humidifier->getVaporizerMode(), humidifier->getFanSpeed());
}

/**
 * \brief Convert milliseconds to in hh:mm:ss
 *
 * \return the time in hh:mm:ss
 */
String HID::convertTime(uint32_t seconds)
{
    char buffer[10];
    int8_t hours = (seconds / 3600) % 24;
    int8_t minutes = (seconds / 60) % 60;
    sprintf(buffer, "%02d:%02d:%02d", hours, minutes, (int) (seconds % 60));
    return String(buffer);
}

/**
 * Convert integer to decimal
 */
String HID::toDecimal(int16_t number, uint8_t divisor)
{
    char buffer[10];
    snprintf(buffer, CFG_LOG_BUFFER_SIZE, "%d.%d", number / divisor, abs(number % divisor));
    return String(buffer);
}

