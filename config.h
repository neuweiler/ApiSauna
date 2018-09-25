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

#define CFG_VERSION                 "ApiSauna v1.1"
#define CFG_DEFAULT_LOGLEVEL        Logger::Info

#define CFG_SERIAL_SPEED 115200
#define CFG_LOOP_DELAY   100

#define CFG_LOG_BUFFER_SIZE         120 // size of log output messages
#define CFG_SERIAL_BUFFER_SIZE      80 // size of the serial input buffer

#define CFG_MAX_NUMBER_PLATES       15 // defines the maximum number of heater plates (limited by 2*x*8 bytes + checksum < 256 bytes)

#endif /* CONFIG_H_ */
