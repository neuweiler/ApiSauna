/*
 * Status.h
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

#ifndef STATUS_H_
#define STATUS_H_

#include <Arduino.h>
#include "Logger.h"
#include "Configuration.h"

class Status
{
public:
    enum SystemState
    {
        init = 1, // the system is being initialized and is not ready for operation yet (next states: ready, error)
        ready = 2, // the system is ready to accept commands but the treatment has not started yet (next states: preHeat, running, error)
        preHeat = 3, // a program was started and the system is heating up the hive (next states: running, error)
        running = 4, // the system is running a program (next states: ready, shutdown, error)
        overtemp = 5, // the hive's temperature is/was too high (next states: shutdown, error)
        shutdown = 9, // the system is shut-down
        error = 99 // the system is in an error state and not operational
    };

    static Status *getInstance();
    SystemState getSystemState();
    SystemState setSystemState(SystemState);
    String systemStateToStr(SystemState);

    int16_t temperatureHive[CFG_MAX_NUMBER_PLATES];
    int16_t temperaturePlate[CFG_MAX_NUMBER_PLATES];
    int16_t temperatureTargetHive;
    int16_t temperatureTargetPlate;
    int16_t temperatureHumidifier;
    uint8_t powerPlate[CFG_MAX_NUMBER_PLATES];
    uint8_t fanSpeedPlate[CFG_MAX_NUMBER_PLATES];
    uint8_t fanSpeedHumidifier;
    bool vaporizerEnabled;
    uint8_t humidity;

private:
    Status();
    Status(Status const&); // copy disabled
    void operator=(Status const&); // assigment disabled

    SystemState systemState; // the current state of the system, to be modified by the state machine of this class only
};

#endif /* STATUS_H_ */
