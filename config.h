/*
 * config.h
 *
 * Defines configuration and static parameters.
 *
 * Note: Make sure with all pin defintions of your hardware that each pin number is
 *       only defined once.

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

#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>

#define CFG_VERSION                 "BeeHive 2017-09-04"
#define CFG_DEFAULT_LOGLEVEL        Logger::Debug

#define CFG_EEPROM_CONFIG_ADDRESS   32
#define CFG_EEPROM_CONFIG_TOKEN     [0xb, 0xee, 0x5, 0xa0, 0x4a]
#define CFG_EEPROM_PROGRAM_ADDRESS  1024

#define CFG_SERIAL_SPEED 115200
#define CFG_LOOP_DELAY   100

#define CFG_PLATES                      4 // defines the number of heater plates (default: 4)
#define CFG_LOG_BUFFER_SIZE             120 // size of log output messages
#define CFG_SERIAL_BUFFER_SIZE          80 // size of the serial input buffer
#define CFG_PROGRAM_SIZE                10 // size of program array

/*
 * Default values for the configuration
 */
#define CFG_IO_BLINK_LED                13 //13 is L, 73 is TX, 72 is RX
#define CFG_IO_TEMPERATURE_SENSOR       4 // pin to which the data line of the single wire temperature sensors are connected
#define CFG_IO_HEATER_MAIN_SWITCH       A0
#define CFG_IO_HEATER_1                 11
#define CFG_IO_HEATER_2                 12
#define CFG_IO_HEATER_3                 2
#define CFG_IO_HEATER_4                 3
#define CFG_IO_FAN_1                    7
#define CFG_IO_FAN_2                    8
#define CFG_IO_FAN_3                    44
#define CFG_IO_FAN_4                    45
#define CFG_IO_VAPORIZER                5
#define CFG_IO_FAN_HUMIDIFIER           6
#define CFG_IO_HUMIDITY_SENSOR          9
#define CFG_IO_LCD_RS                   26
#define CFG_IO_LCD_ENABLE               27
#define CFG_IO_LCD_D0                   22
#define CFG_IO_LCD_D1                   23
#define CFG_IO_LCD_D2                   24
#define CFG_IO_LCD_D3                   25
#define CFG_IO_LCD_BUTTON               A1

#define CFG_ADDR_TEMP_SENSOR_1          0x3d0516a4f187ff28
#define CFG_ADDR_TEMP_SENSOR_2          0x9a0516a50124ff28
#define CFG_ADDR_TEMP_SENSOR_3          0xe10316a5188cff28
#define CFG_ADDR_TEMP_SENSOR_4          0xed0316a48290ff28

#define CFG_MAX_HEATER_POWER            170
#define CFG_MIN_FAN_SPEED               10
#define CFG_HIVE_OVER_TEMPERATURE       480
#define CFG_HIVE_TEMPERATURE_RECOVER    350
#define CFG_PLATE_OVER_TEMPERATURE      800

class Configuration
{
public:
    uint8_t checksum;

    uint8_t temperatureSensorPin;
    uint8_t humiditySensorPin;
    uint8_t vaporizerPin;
    uint8_t humidifierFanPin;
    uint8_t heaterRelayPin;
    uint8_t heaterPin[CFG_PLATES]; // the pin assignments of the heaters
    uint8_t fanPin[CFG_PLATES]; // the pin assigments of the plate fans
    uint64_t sensorAddress[CFG_PLATES]; // the addresses of the heater sensors assigned to the plates

    uint8_t maxHeaterPower;
    uint8_t minFanSpeed;
    uint8_t hiveOverTemp;
    uint8_t hiveOverTempRecover;
    uint8_t plateOverTemp;
};

extern Configuration config;

#endif /* CONFIG_H_ */
