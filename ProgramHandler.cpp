/*
 * Program.cpp
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

#include "ProgramHandler.h"

ProgramHandler::ProgramHandler()
{
    runningProgram = NULL;
    startTime = 0;
}

ProgramHandler::~ProgramHandler()
{
}

ProgramHandler *ProgramHandler::getInstance()
{
    static ProgramHandler instance;
    return &instance;
}

/**
 * Initialize all programs
 */
void ProgramHandler::initPrograms()
{
    Logger::info("Loading program data");

    Program programVarroaSummer;
    snprintf(programVarroaSummer.name, 16, "Varroa Killer");
    programVarroaSummer.temperaturePreHeat = 420; // 42 deg C
    programVarroaSummer.fanSpeedPreHeat = 250;
    programVarroaSummer.durationPreHeat = 60; // 60min
    programVarroaSummer.temperatureHive = 430; // 430 deg C
    programVarroaSummer.hiveKp = 8.0;
    programVarroaSummer.hiveKi = 0.2;
    programVarroaSummer.hiveKd = 5.0;
    programVarroaSummer.temperaturePlate = 700; // 70 deg C
    programVarroaSummer.plateKp = 3.0;
    programVarroaSummer.plateKi = 0.01;
    programVarroaSummer.plateKd = 50.0;
    programVarroaSummer.fanSpeed = 200; // minimum is 10
    programVarroaSummer.humidityMinimum = 30;
    programVarroaSummer.humidityMaximum = 35;
    programVarroaSummer.fanSpeedHumidifier = 240; // only works from 230 to 255
    programVarroaSummer.duration = 210; // 3.5 hours
    programs.push_back(programVarroaSummer);

    Program programVarroaWinter;
    snprintf(programVarroaWinter.name, 16, "Winter Treat");
    programVarroaWinter.temperaturePreHeat = 420; // 42 deg C
    programVarroaWinter.fanSpeedPreHeat = 255;
    programVarroaWinter.durationPreHeat = 60; // 60min
    programVarroaWinter.temperatureHive = 430; // 43.0 deg C
    programVarroaWinter.hiveKp = 8.0;
    programVarroaWinter.hiveKi = 0.2;
    programVarroaWinter.hiveKd = 5.0;
    programVarroaWinter.temperaturePlate = 750; // 75 deg C
    programVarroaWinter.plateKp = 4.0;
    programVarroaWinter.plateKi = 0.09;
    programVarroaWinter.plateKd = 50.0;
    programVarroaWinter.fanSpeed = 255; // minimum is 10
    programVarroaWinter.humidityMinimum = 30;
    programVarroaWinter.humidityMaximum = 35;
    programVarroaWinter.fanSpeedHumidifier = 240; // only works from 230 to 255
    programVarroaWinter.duration = 180; // 3 hours
    programs.push_back(programVarroaWinter);

    Program programCleaning;
    snprintf(programCleaning.name, 16, "Cleaning");
    programCleaning.temperaturePreHeat = 380; // 38 deg C
    programCleaning.fanSpeedPreHeat = 0;
    programCleaning.durationPreHeat = 0; // 30min
    programCleaning.temperatureHive = 425; // 42.5 deg C
    programCleaning.hiveKp = 8.0;
    programCleaning.hiveKi = 0.2;
    programCleaning.hiveKd = 5.0;
    programCleaning.temperaturePlate = 600; // 60 deg C
    programCleaning.plateKp = 4.0;
    programCleaning.plateKi = 0.09;
    programCleaning.plateKd = 50.0;
    programCleaning.fanSpeed = 0; // minimum is 10
    programCleaning.humidityMinimum = 1;
    programCleaning.humidityMaximum = 2;
    programCleaning.fanSpeedHumidifier = 0; // only works from 230 to 255
    programCleaning.duration = 15; // 15min
    programs.push_back(programCleaning);

    Program programMeltHoney;
    snprintf(programMeltHoney.name, 16, "Melt Honey");
    programMeltHoney.temperaturePreHeat = 300;
    programMeltHoney.fanSpeedPreHeat = 0;
    programMeltHoney.durationPreHeat = 0;
    programMeltHoney.temperatureHive = 300;
    programMeltHoney.hiveKp = 8.0;
    programMeltHoney.hiveKi = 0.2;
    programMeltHoney.hiveKd = 5.0;
    programMeltHoney.temperaturePlate = 500;
    programMeltHoney.plateKp = 4.0;
    programMeltHoney.plateKi = 0.09;
    programMeltHoney.plateKd = 50.0;
    programMeltHoney.fanSpeed = 0;
    programMeltHoney.humidityMinimum = 1;
    programMeltHoney.humidityMaximum = 2;
    programMeltHoney.fanSpeedHumidifier = 0; // only works from 230 to 255
    programMeltHoney.duration = 720; // 12 hours
    programs.push_back(programMeltHoney);

//    ControllerProgram programWellness; 38°, 3h
//    ControllerProgram programHealth; 39° 30min
}

Program* ProgramHandler::getRunningProgram()
{
    return runningProgram;
}

SimpleList<Program>* ProgramHandler::getPrograms()
{
    return &programs;
}

void ProgramHandler::start(uint8_t programNumber)
{
    int i = 1;
    SimpleList<Program>* programs = ProgramHandler::getInstance()->getPrograms();
    for (SimpleList<Program>::iterator itr = programs->begin(); itr != programs->end(); ++itr) {
        if (i == programNumber) {
            Logger::info(F("Starting program #%d"), i);
                runningProgram = itr;
                sendEvent(startProgram, runningProgram);
                startTime = millis();
                return;
        }
        i++;
    }
    Logger::warn(F("program #%d not found"), programNumber);
}

void ProgramHandler::stop()
{
    Logger::info(F("stopping program"));
    runningProgram = NULL;
    sendEvent(stopProgram, runningProgram);
    Status::getInstance()->setSystemState(Status::shutdown);
}

uint32_t ProgramHandler::getTimeRunning()
{
    Status::SystemState state = Status::getInstance()->getSystemState();
    if (runningProgram != NULL && (state == Status::preHeat || state == Status::running)) {
        return (millis() - startTime) / 1000;
    }
    return 0;
}

/**
 * Returns the time the current program stage is running in seconds.
 */
uint32_t ProgramHandler::getTimeRemaining()
{
    if (runningProgram != NULL) {
        uint32_t timeRunning = getTimeRunning();
        uint32_t duration = (Status::getInstance()->getSystemState() == Status::preHeat ? runningProgram->durationPreHeat : runningProgram->duration) * 60;
        // perform a safety check to prevent an unsigned under-flow if timeRunning is bigger than the duration
        if (timeRunning < duration) {
            return duration - timeRunning;
        }
    }
    return 0;
}

void ProgramHandler::attach(ProgramObserver *observer) {
    observers.push_back(observer);
}

void ProgramHandler::sendEvent(ProgramEvent event, Program *program) {
    Logger::debug(F("sending event %d to observers of ProgramHandler"), event);
    for (SimpleList<ProgramObserver *>::iterator itr = observers.begin(); itr != observers.end(); ++itr) {
        ((ProgramObserver *)*itr)->handleEvent(event, program);
    }
}
