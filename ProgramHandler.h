/*
 * Program.h
 *
 Copyright (c) 2018 Michael Neuweiler

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

#ifndef PROGRAMHANDLER_H_
#define PROGRAMHANDLER_H_

#include <Arduino.h>
#include "Logger.h"
#include "SimpleList.h"
#include "Status.h"

class Program
{
public:
    char name[17]; // name to be displayed in menu
    int16_t temperaturePreHeat; // the target hive temperature during pre-heat (in 0.1 deg C)
    int16_t temperatureHive; // the target hive temperature (in 0.1 deg C)
    double hiveKp, hiveKi, hiveKd; // hive temperature PID configuration
    int16_t temperaturePlate; // the target temperature of the heater plates (in 0.1 deg C)
    double plateKp, plateKi, plateKd; // plate temperature PID configuration
    uint8_t fanSpeedPreHeat; // the fan speed during pre-heat (0-255)
    uint8_t fanSpeed; // the fan speed (0-255)
    uint16_t durationPreHeat; // the duration of the pre-heating cycle (in min)
    uint16_t duration; // the duration of the program (in min)
    uint8_t fanSpeedHumidifier; // the fan speed of the humidifier fan (when active (0-255)
    uint8_t humidityMinimum; // the minimum relative humidity in %
    uint8_t humidityMaximum; // the maximum relative humidity in %

    bool changed; // the program's values were changed indicating a required update
};

enum ProgramEvent
{
    startProgram,
    stopProgram
};

class ProgramObserver
{
public:
    virtual void handleEvent(ProgramEvent event, Program *program);
};

class ProgramHandler
{
public:
    static ProgramHandler *getInstance();
    virtual ~ProgramHandler();
    void initPrograms();
    SimpleList<Program> *getPrograms();
    void start(uint8_t programNumber);
    void stop();
    Program *getRunningProgram();
    uint32_t getTimeRunning();
    uint32_t getTimeRemaining();
    void attach(ProgramObserver *observer);

private:
    ProgramHandler();
    ProgramHandler(ProgramHandler const&); // copy disabled
    void operator=(ProgramHandler const&); // assigment disabled    SimpleList<Program> programs;
    void sendEvent(ProgramEvent event, Program *program);

    Program *runningProgram;
    SimpleList<Program> programs;
    SimpleList<ProgramObserver *> observers;
    uint32_t startTime; // timestamp when the program started (in millis)

};

#endif /* PROGRAMHANDLER_H_ */
