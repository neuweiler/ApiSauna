/*
 * Humidifier.h
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

#ifndef HUMIDIFIER_H_
#define HUMIDIFIER_H_

#include "Device.h"
#include "HumiditySensor.h"
#include "Fan.h"
#include "Vaporizer.h"

class Humidifier: Device
{
public:
    Humidifier();
    void initialize();
    void process();
    void setMaxHumidity(uint8_t maxHumidity);
    uint8_t getMaxHumidity();
    void setMinHumidity(uint8_t minHumidity);
    uint8_t getMinHumidity();
    void setFanSpeed(uint8_t speed);
    uint8_t getFanSpeed();
    uint8_t getHumidity();
    int16_t getTemperature();
    Vaporizer::Mode getVaporizerMode();

private:
    Vaporizer vaporizer;
    HumiditySensor sensor;
    Fan fan;
    uint8_t maximumHumidity;
    uint8_t minimumHumidity;
    uint8_t humidity;
    int16_t temperature;
    uint8_t fanSpeed;
};

#endif /* HUMIDIFIER_H_ */
