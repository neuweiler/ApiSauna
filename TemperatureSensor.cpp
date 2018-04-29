/*
 * TemperatureSensor.cpp
 *
 * High-level abstraction of the temperature sensor hardware.
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

#include "TemperatureSensor.h"

OneWire *TemperatureSensor::ds = NULL;

/**
 * Constructor
 */
TemperatureSensor::TemperatureSensor()
{
    index = 0;
    temperature = 0;
    type = UNKNOWN;
}

/**
 * Constructor
 */
TemperatureSensor::TemperatureSensor(uint8_t index, bool plate)
{
    temperature = 0;
    setAddress((plate ? Configuration::getSensor()->addressPlate[index] : Configuration::getSensor()->addressHive[index]));
    if (ds == NULL) {
        ds = new OneWire(Configuration::getIO()->temperatureSensor);
    }
}

/**
 * Get the type of the device
 */
TemperatureSensor::DeviceType TemperatureSensor::getType()
{
    return type;
}

/**
 * Get the type of the device as string
 */
String TemperatureSensor::getTypeStr()
{
    switch (type) {
    case TemperatureSensor::DS18S20:
        return F("DS18S20");
        break;
    case TemperatureSensor::DS18B20:
        return F("DS18B20");
        break;
    case TemperatureSensor::DS1822:
        return F("DS1822");
        break;
    default:
        return F("unknown");
        break;
    }
}

/**
 * Get the address of the device
 */
SensorAddress TemperatureSensor::getAddress()
{
    return address;
}

/**
 * Set the address of the device
 */
void TemperatureSensor::setAddress(SensorAddress address)
{
    this->address.value = address.value;
    if (address.byte[0] == 0x10) {
        type = DS18S20;
    } else if (address.byte[0] == 0x28) {
        type = DS18B20;
    } else if (address.byte[0] == 0x22) {
        type = DS1822;
    } else {
        type = UNKNOWN;
    }
}

/**
 * Set the resolution of the DS18B20 between 9 or 12 bits.
 */
void TemperatureSensor::setResolution(byte resolution)
{
    if (resolution > 12 || resolution < 9)
        return;
    if (type != DS18B20)
        return;

    // Get byte for desired resolution
    byte resolutionByte = 0x1F; // 9 bit
    if (resolution == 12) {
        resolutionByte = 0x7F;
    } else if (resolution == 11) {
        resolutionByte = 0x5F;
    } else if (resolution == 10) {
        resolutionByte = 0x3F;
    }

    // set configuration
    ds->reset();
    ds->select(address.byte);
    ds->write(0x4E);			// write scratchpad
    ds->write(0);				// TL
    ds->write(0);				// TH
    ds->write(resolutionByte);	// configuration register
    ds->write(0x48);			// copy scratchpad
}

/**
 * Order all temperature sensors to prepare data.
 */
void TemperatureSensor::prepareData()
{
    ds->reset();
    ds->skip(); // skip ROM - send to all devices
    ds->write(0x44); // start conversion
}

/**
 * Retrieve prepared data from temperature sensors
 */
void TemperatureSensor::retrieveData()
{
    byte data[9];

    ds->reset();
    ds->select(address.byte);
    ds->write(0xBE); // read scratchpad
    ds->read_bytes(data, 9); // 9 bytes are required

    temperature = (data[1] << 8) | data[0];

    if (type == DS18S20) {
        temperature = temperature << 3; // 9 bit resolution default
        if (data[7] == 0x10) { // "count remain" gives full 12 bit resolution
            temperature = (temperature & 0xFFF0) + 12 - data[6];
        }
    } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
            temperature = temperature & ~7; // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
            temperature = temperature & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
            temperature = temperature & ~1; // 11 bit res, 375 ms
    }
}

/**
 * Return the measured temperature in 0.1 degree celsius
 */
int16_t TemperatureSensor::getTemperatureCelsius()
{
    return temperature * 5 / 8;
}

/**
 * Return the measured temperature in 0.1 degree fahrenheit
 */
int16_t TemperatureSensor::getTemperatureFahrenheit()
{
    return temperature * 9 / 8 + 320;
}

/**
 * A static wrapper for the OneWire::reset_search()
 */
void TemperatureSensor::resetSearch()
{
    if (ds == NULL) {
        ds = new OneWire(Configuration::getIO()->temperatureSensor);
    }
    ds->reset_search();
}

/**
 * A static function to search for more devices.
 * If no more are found, the address' value is 0.
 */
SensorAddress TemperatureSensor::search()
{
    SensorAddress addr;

    addr.value = 0;
    if (ds->search(addr.byte)) {
        if (OneWire::crc8(addr.byte, 7) != addr.byte[7]) {
            Logger::error(F("temperature sensor: invalid CRC!\n"));
            addr.value = 0;
        }
    }
    return addr;
}
