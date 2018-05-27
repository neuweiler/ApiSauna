/*
 * Status.cpp
 *
 * Holds the state machine's current status and transition logic as well as
 * some global status values.
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

#include "Status.h"

/*
 * Constructor
 */
Status::Status()
{
    systemState = init;
    errorCode = none;
    for (int i = 0; i < CFG_MAX_NUMBER_PLATES; i++) {
        temperatureHive[i] = 0;
        temperaturePlate[i] = 0;
        powerPlate[i] = 0;
        fanSpeedPlate[i] = 0;
    }
    temperatureTargetHive = 0;
    temperatureTargetPlate = 0;
    temperatureHumidifier = 0;
    fanSpeedHumidifier = 0;
    fanTimeHumidifier = 0;
    vaporizerEnabled = false;
    humidity = 0;

}

Status *Status::getInstance() {
    static Status instance;
    return &instance;
}

/*
 * Retrieve the current system state.
 */
Status::SystemState Status::getSystemState()
{
    return systemState;
}

/*
 * Set a new system state. The new system state is validated if the
 * transition is allowed from the old state. If an invalid transition is
 * attempted, the new state will be 'error'.
 * The old and new state are broadcast to all devices.
 */
Status::SystemState Status::setSystemState(SystemState newSystemState)
{
    if (systemState == newSystemState) {
        return systemState;
    }

    if (newSystemState == error || newSystemState == overtemp) { // go in these states unconditionally!
        systemState = newSystemState;
    } else {
        switch (systemState) {
        case init:
            if (newSystemState == ready) {
                systemState = newSystemState;
            }
            break;
        case ready:
            if (newSystemState == preHeat || newSystemState == running || newSystemState == shutdown) {
                systemState = newSystemState;
            }
            break;
        case shutdown:
            if (newSystemState == preHeat || newSystemState == running) {
                systemState = newSystemState;
            }
            break;
        case preHeat:
            if (newSystemState == running || newSystemState == shutdown) {
                systemState = newSystemState;
            }
            break;
        case running:
            if (newSystemState == ready || newSystemState == shutdown) {
                systemState = newSystemState;
            }
            break;
        case overtemp:
            if (newSystemState == shutdown) {
                systemState = newSystemState;
            }
            break;
        }
    }
    if (systemState == newSystemState) {
        Logger::info(F("switching to state '%s'"), systemStateToStr(systemState).c_str());
    } else {
        Logger::error(F("switching from state '%s' to '%s' is not allowed"), systemStateToStr(systemState).c_str(), systemStateToStr(newSystemState).c_str());
        systemState = error;
        errorCode = invalidState;
    }

    return systemState;
}

/*
 * Convert the state into a string.
 */
String Status::systemStateToStr(SystemState state)
{
    switch (state) {
    case init:
        return F("initializing");
    case ready:
        return F("ready");
    case preHeat:
        return F("pre-heating");
    case running:
        return F("running");
    case overtemp:
        return F("over-temperature");
    case shutdown:
        return F("shut-down");
    case error:
        return F("error");
    }
    return F("invalid");
}

/*
 * Convert the current state into a string.
 */
String Status::errorCodeToStr(ErrorCode code)
{
    switch (code) {
    case none:
        return F("no error");
    case crcParam:
        return F("Param config invalid CRC");
    case crcIo:
        return F("I/O config invalid CRC");
    case crcSensor:
        return F("Sensor config invalid CRC");
    case crcStatistics:
        return F("Statistics invalid CRC");
    case plateSensorsNotFound:
        return F("Not all plate sensors found");
    case hiveSensorsNotFound:
        return F("Not all hive sensors found");
    case overtempHive:
        return F("Hive over-temp");
    case overtempPlate:
        return F("Plate over-temp");
    case invalidState:
        return F("Invalid state switch");
    }
    return F("n/a");
}
