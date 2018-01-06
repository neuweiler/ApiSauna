/*
 * SerialConsole.cpp
 *
 * Serial interface to influence the behaviour and chang the contifuration
 * of the bee sauna.
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

#include "SerialConsole.h"

SerialConsole::SerialConsole()
{
    ptrBuffer = 0;
}

SerialConsole::~SerialConsole()
{
}

void SerialConsole::loop()
{
    while (Serial.available()) {
        int incoming = Serial.read();

        if (incoming == -1) { //false alarm....
            return;
        }

        if (incoming == 10 || incoming == 13) { //command done. Parse it.
            if (ptrBuffer == 1) {
                handleShortCmd();
            } else {
                handleCmd();
            }
            ptrBuffer = 0; //reset line counter once the line has been processed
        } else {
            cmdBuffer[ptrBuffer++] = (unsigned char) incoming;

            if (ptrBuffer > CFG_SERIAL_BUFFER_SIZE) {
                ptrBuffer = CFG_SERIAL_BUFFER_SIZE;
            }
        }
    }
}

void SerialConsole::printMenu()
{
    //Show build # here as well in case people are using the native port and don't get to see the start up messages
    Logger::console("\n%s", CFG_VERSION);
    Logger::console("System State: %s", status.systemStateToStr(status.getSystemState()).c_str());
    Logger::console("System Menu:\n");
    Logger::console("Enable line endings of some sort (LF, CR, CRLF)\n");
    Logger::console("Commands:");
    Logger::console("h = help (displays this message)");
    Logger::console("x = stop program");
    Logger::console("start=<program #> - start program number");

    Logger::console("\nConfig Commands (enter command=newvalue)\n");
    Logger::console("LOGLEVEL=%d - set log level (0=debug, 1=info, 2=warn, 3=error, 4=off)", Logger::getLogLevel());

    ControllerProgram *program = controller.getRunningProgram();
    if (program) {
        Logger::console("\nPROGRAM\n");
        Logger::console("TEMP-PREHEAT=%d - pre-heat hive temperature (in 0.1 deg C, 0-600)", program->temperaturePreHeat);
        Logger::console("TEMP=%d - hive temperature (in 0.1 deg C, 0-600)", program->temperatureHive);
        Logger::console("TEMP-PLATE=%d - max plate temperature (in 0.1 deg C, 0-1000)", program->temperaturePlate);
        Logger::console("FANSPEED-PREHEAT=%d - fan speed during pre-heat (0-255)", program->fanSpeedPreHeat);
        Logger::console("FANSPEED=%d - fan speed (0-255)", program->fanSpeed);
        Logger::console("FANSPEED-HUMID=%d - fan speed of humidifier (0-255)", program->fanSpeedHumidifier);
        Logger::console("HUMIDITY-MIN=%d - relative humidity minimum (0-100)", program->humidityMinimum);
        Logger::console("HUMIDITY-MAX=%d - relative humidity maximum (0-100)", program->humidityMaximum);
        Logger::console("DURATION-PREHEAT=%d - duration of pre-heat cycle (in min)", program->durationPreHeat);
        Logger::console("DURATION=%d - duration of program (in min)", program->duration);
        Logger::console("enter the following values multiplied by 100 (e.g. 25 for 0.25) :");
        Logger::console("HIVE-KP=%s - Kp parameter for hive temperature PID", String(program->hiveKp).c_str());
        Logger::console("HIVE-KI=%s - Ki parameter for hive temperature PID", String(program->hiveKi).c_str());
        Logger::console("HIVE-KD=%s - Kd parameter for hive temperature PID", String(program->hiveKd).c_str());
        Logger::console("PLATE-KP=%s - Kp parameter for plate temperature PID", String(program->plateKp).c_str());
        Logger::console("PLATE-KI=%s - Ki parameter for plate temperature PID", String(program->plateKi).c_str());
        Logger::console("PLATE-KD=%s - Kd parameter for plate temperature PID", String(program->plateKd).c_str());
        Logger::console("");
    }
}

bool SerialConsole::handleCmd()
{
    int i = 0;

    if (ptrBuffer < 1) {
        return false;
    }

    cmdBuffer[ptrBuffer] = 0; //make sure to null terminate
    String command = String();

    while (cmdBuffer[i] != '=' && i < ptrBuffer) {
        command.concat(String(cmdBuffer[i++]));
    }
    i++;

    if (i >= ptrBuffer) {
        Logger::console("Command needs a value..ie TEMP=420\n");
        return false;
    }

    int32_t value = strtol((char *) (cmdBuffer + i), NULL, 0);
    command.toUpperCase();

    if (command == String("LOGLEVEL")) {
        value = constrain(value, 0, 4);
        Logger::console("Setting loglevel to %d", value);
        Logger::setLoglevel((Logger::LogLevel) value);
    }

    ControllerProgram *program = controller.getRunningProgram();
    if (program == NULL) {
        if (command == String("START")) {
            Logger::info("starting program #%d", value);
            controller.startProgram(value);
        } else {
            return false;
        }
    } else {
        if (command == String("TEMP-PREHEAT")) {
            value = constrain(value, 0, 600);
            Logger::console("Setting pre-heat hive temperature to %d.%d deg C", value / 10, value % 10);
            program->temperaturePreHeat = value;
        } else if (command == String("TEMP")) {
            value = constrain(value, 0, 600);
            Logger::console("Setting hive temperature to %d.%d deg C", value / 10, value % 10);
            program->temperatureHive = value;
        } else if (command == String("TEMP-PLATE")) {
            value = constrain(value, 0, 1000);
            Logger::console("Setting max. plate temperature to %d.%d deg C", value / 10, value % 10);
            program->temperaturePlate = value;
        } else if (command == String("FANSPEED-PREHEAT")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting pre-heat fan speed to %d", value);
            program->fanSpeedPreHeat = value;
        } else if (command == String("FANSPEED")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting fan speed to %d", value);
            program->fanSpeed = value;
            controller.setFanSpeed(value);
        } else if (command == String("FANSPEED-HUMID")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting speed of humidifier fan to %d", value);
            program->fanSpeedHumidifier = value;
            controller.setFanSpeedHumidifier(value);
        } else if (command == String("HUMIDITY-MIN")) {
            value = constrain(value, 0, 100);
            Logger::console("Setting relative humidity minimum to %d %%", value);
            program->humidityMinimum = value;
            controller.setHumidifierLimits(program->humidityMinimum, program->humidityMaximum);
        } else if (command == String("HUMIDITY-MAX")) {
            value = constrain(value, 0, 100);
            Logger::console("Setting relative humidity maximum to %d %%", value);
            program->humidityMaximum = value;
            controller.setHumidifierLimits(program->humidityMinimum, program->humidityMaximum);
        } else if (command == String("DURATION-PREHEAT")) {
            value = constrain(value, 0, 0xffff);
            Logger::console("Setting duration of pre-heat cycle to %d min", value);
            program->durationPreHeat = value;
        } else if (command == String("DURATION")) {
            value = constrain(value, 0, 0xffff);
            Logger::console("Setting duration of program to %d min", value);
            program->duration = value;
        } else if (command == String("HIVE-KP")) {
            program->hiveKp = (double)value / (double)100.0;
            Logger::console("Setting hive temperature Kp to %s", String(program->hiveKp).c_str());
            controller.setPIDTuningHive(program->hiveKp, program->hiveKi, program->hiveKd);
        } else if (command == String("HIVE-KI")) {
            program->hiveKi = (double)value / (double)100.0;
            Logger::console("Setting hive temperature Ki to %s", String(program->hiveKi).c_str());
            controller.setPIDTuningHive(program->hiveKp, program->hiveKi, program->hiveKd);
        } else if (command == String("HIVE-KD")) {
            program->hiveKd = (double)value / (double)100.0;
            Logger::console("Setting hive temperature Kd to %s", String(program->hiveKd).c_str());
            controller.setPIDTuningHive(program->hiveKp, program->hiveKi, program->hiveKd);
        } else if (command == String("PLATE-KP")) {
            program->plateKp = (double)value / (double)100.0;
            Logger::console("Setting plate temperature Kp to %s", String(program->plateKp).c_str());
            controller.setPIDTuningPlate(program->plateKp, program->plateKi, program->plateKd);
        } else if (command == String("PLATE-KI")) {
            program->plateKi = (double)value / (double)100.0;
            Logger::console("Setting plate temperature Ki to %s", String(program->plateKi).c_str());
            controller.setPIDTuningPlate(program->plateKp, program->plateKi, program->plateKd);
        } else if (command == String("PLATE-KD")) {
            program->plateKd = (double)value / (double)100.0;
            Logger::console("Setting plate temperature Kd to %s", String(program->plateKd).c_str());
            controller.setPIDTuningPlate(program->plateKp, program->plateKi, program->plateKd);
        } else {
            return false;
        }
    }
//    saveConfiguration();
    return true;
}

bool SerialConsole::handleShortCmd()
{
    switch (cmdBuffer[0]) {
    case 'h':
    case '?':
    case 'H':
        printMenu();
        break;

    case 'x':
        Logger::info("Stopping program");
        controller.stopProgram();
        break;

    default:
        return false;
        break;
    }
    return true;
}
