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

/**
 * Get singleton instance
 */
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
    Logger::info(F("Loading program data"));

    Program programVarroaSummer;
    snprintf(programVarroaSummer.name, 16, "Varroa Killer");
    programVarroaSummer.temperaturePreHeat = 400; // 40 deg C
    programVarroaSummer.fanSpeedPreHeat = 250;
    programVarroaSummer.durationPreHeat = 60; // 60min
    programVarroaSummer.temperatureHive = 410; // 41 deg C
    programVarroaSummer.hiveKp = 4.0;
    programVarroaSummer.hiveKi = 0.2;
    programVarroaSummer.hiveKd = 7.0;
    programVarroaSummer.temperaturePlate = 700; // 70 deg C
    programVarroaSummer.plateKp = 1.0;
    programVarroaSummer.plateKi = 0.10;
    programVarroaSummer.plateKd = 7.0;
    programVarroaSummer.fanSpeed = 200; // minimum is 10
    programVarroaSummer.humidityMinimum = 30;
    programVarroaSummer.humidityMaximum = 35;
    programVarroaSummer.fanSpeedHumidifier = 240; // only works from 230 to 255
    programVarroaSummer.duration = 210; // 3.5 hours
    programs.push_back(programVarroaSummer);

    Program programVarroaWinter;
    snprintf(programVarroaWinter.name, 16, "Winter Treat");
    programVarroaWinter.temperaturePreHeat = 400; // 40 deg C
    programVarroaWinter.fanSpeedPreHeat = 255;
    programVarroaWinter.durationPreHeat = 60; // 60min
    programVarroaWinter.temperatureHive = 420; // 41.0 deg C
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
    programCleaning.fanSpeedPreHeat = 10;
    programCleaning.durationPreHeat = 0;
    programCleaning.temperatureHive = 425; // 42.5 deg C
    programCleaning.hiveKp = 8.0;
    programCleaning.hiveKi = 0.2;
    programCleaning.hiveKd = 5.0;
    programCleaning.temperaturePlate = 600; // 60 deg C
    programCleaning.plateKp = 4.0;
    programCleaning.plateKi = 0.09;
    programCleaning.plateKd = 50.0;
    programCleaning.fanSpeed = 10; // minimum is 10
    programCleaning.humidityMinimum = 1;
    programCleaning.humidityMaximum = 2;
    programCleaning.fanSpeedHumidifier = 0; // only works from 230 to 255
    programCleaning.duration = 15; // 15min
    programs.push_back(programCleaning);

    Program programMeltHoney;
    snprintf(programMeltHoney.name, 16, "Melt Honey");
    programMeltHoney.temperaturePreHeat = 300;
    programMeltHoney.fanSpeedPreHeat = 10;
    programMeltHoney.durationPreHeat = 0;
    programMeltHoney.temperatureHive = 300;
    programMeltHoney.hiveKp = 8.0;
    programMeltHoney.hiveKi = 0.2;
    programMeltHoney.hiveKd = 5.0;
    programMeltHoney.temperaturePlate = 500;
    programMeltHoney.plateKp = 4.0;
    programMeltHoney.plateKi = 0.09;
    programMeltHoney.plateKd = 50.0;
    programMeltHoney.fanSpeed = 10;
    programMeltHoney.humidityMinimum = 1;
    programMeltHoney.humidityMaximum = 2;
    programMeltHoney.fanSpeedHumidifier = 0; // only works from 230 to 255
    programMeltHoney.duration = 720; // 12 hours
    programs.push_back(programMeltHoney);
}

/**
 * Returns the address of the currently running program
 */
Program* ProgramHandler::getRunningProgram()
{
    return runningProgram;
}

/**
 * Returns the list of all defined programs
 */
SimpleList<Program>* ProgramHandler::getPrograms()
{
    return &programs;
}

/**
 * Start a specific program
 */
void ProgramHandler::start(uint8_t programNumber)
{
    int i = 1;
    for (SimpleList<Program>::iterator itr = programs.begin(); itr != programs.end(); ++itr) {
        if (i == programNumber) {
            Logger::info(F("Starting program #%d"), i);
            runningProgram = itr;
            runningProgram->changed = false;
            startTime = millis();
            status.setSystemState(runningProgram->durationPreHeat == 0 ? Status::running : Status::preHeat);
            sendEvent(startProgram, runningProgram);
            return;
        }
        i++;
    }
    Logger::warn(F("program #%d not found"), programNumber);
}

/**
 * Stop the currently running program
 */
void ProgramHandler::stop()
{
    Logger::info(F("stopping program"));
    startTime = 0;
    status.setSystemState(Status::shutdown);
    sendEvent(stopProgram, runningProgram);
}

/**
 * Pause the currently running program
 */
void ProgramHandler::pause()
{
    Logger::info(F("pausing program"));
    sendEvent(pauseProgram, runningProgram);
}

/**
 * Resume the currently running program
 */
void ProgramHandler::resume()
{
    Logger::info(F("resuming program"));
    sendEvent(resumeProgram, runningProgram);
}

void ProgramHandler::addTime(uint16_t duration) {
    Program *clone = new Program();
    clone->duration = duration;
    clone->fanSpeed = runningProgram->fanSpeed;
    clone->fanSpeedHumidifier = runningProgram->fanSpeedHumidifier;
    clone->hiveKp = runningProgram->hiveKp;
    clone->hiveKi = runningProgram->hiveKi;
    clone->hiveKd = runningProgram->hiveKd;
    clone->humidityMinimum = runningProgram->humidityMinimum;
    clone->humidityMaximum = runningProgram->humidityMaximum;
    strcpy(clone->name, runningProgram->name);
    clone->plateKp = runningProgram->plateKp;
    clone->plateKi = runningProgram->plateKi;
    clone->plateKd = runningProgram->plateKd;
    clone->temperatureHive = runningProgram->temperatureHive;
    clone->temperaturePlate = runningProgram->temperaturePlate;
    runningProgram = clone;

    Logger::info(F("extending program %s by %dmin"), runningProgram->name, duration);
    startTime = millis();
    status.setSystemState(Status::ready);
    status.setSystemState(Status::running);
    sendEvent(startProgram, runningProgram);
}

/**
 * When switching from pre-heat to running, we need to reset the start time. (to start with time running == 0)
 */
void ProgramHandler::switchToRunning() {
    startTime = millis();
    status.setSystemState(Status::running);
    sendEvent(updateProgram, runningProgram);
}

/**
 * Calculate the time the pre-heat or running phase is running (in seconds)
 */
uint32_t ProgramHandler::calculateTimeRunning()
{
    if (startTime != 0) {
        return (millis() - startTime) / 1000;
    }
    return 0;
}

/**
 * Calculate the time the current program stage is running (in seconds).
 */
uint32_t ProgramHandler::calculateTimeRemaining()
{
    if (runningProgram != NULL) {
        uint32_t timeRunning = calculateTimeRunning();
        uint32_t duration = (status.getSystemState() == Status::preHeat ? runningProgram->durationPreHeat : runningProgram->duration) * 60;
        if (timeRunning < duration) { // prevent under-flow
            return duration - timeRunning;
        }
    }
    return 0;
}

/**
 * Attach a program observer so it will get notified of future changes in program selection
 */
void ProgramHandler::attach(ProgramObserver *observer) {
    observers.push_back(observer);
}

/**
 * Send an event to all attached/subscribed listeners
 */
void ProgramHandler::sendEvent(ProgramEvent event, Program *program) {
    Logger::debug(F("sending event %d to observers of ProgramHandler"), event);
    for (SimpleList<ProgramObserver *>::iterator itr = observers.begin(); itr != observers.end(); ++itr) {
        ((ProgramObserver *)*itr)->handleEvent(event, program);
    }
}
