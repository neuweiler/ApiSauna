/*
 * config.h
 *
 * Defines the components to be used in the BeeSauna and allows the user to configure
 * static parameters.
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

#define CFG_VERSION "BeeHive 2017-08-22"
#define CFG_DEFAULT_LOGLEVEL Logger::Info

/*
 * SERIAL CONFIGURATION
 */
#define CFG_SERIAL_SPEED 115200
//#define Serial SerialUSB // re-route serial output to serial-usb

/*
 * TIMER INTERVALS
 *
 */
#define CFG_LOOP_DELAY   1000

/*
 * ARRAY SIZE
 *
 * Define the maximum number of various object lists.
 * These values should normally not be changed.
 */
#define CFG_LOG_BUFFER_SIZE             120 // size of log output messages
#define CFG_SERIAL_BUFFER_SIZE          80 // size of the serial input buffer
#define CFG_PROGRAM_SIZE                10 // size of program array

/*
 * PIN ASSIGNMENT
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

#define CFG_ADDR_TEMP_SENSOR_1          0x3d0516a4f187ff28
#define CFG_ADDR_TEMP_SENSOR_2          0x9a0516a50124ff28
#define CFG_ADDR_TEMP_SENSOR_3          0xe10316a5188cff28
#define CFG_ADDR_TEMP_SENSOR_4          0xed0316a48290ff28

#define CFG_MAX_HEATER_POWER            170
#define CFG_MIN_FAN_SPEED               10
#define CFG_HIVE_OVER_TEMPERATURE       480
#define CFG_HIVE_TEMPERATURE_RECOVER    350
#define CFG_PLATE_OVER_TEMPERATURE      800

#endif /* CONFIG_H_ */
