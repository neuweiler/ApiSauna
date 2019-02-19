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

SerialConsole::SerialConsole() :
        Device()
{
    ptrBuffer = 0;
}

void SerialConsole::process()
{
    Device::process();

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

/**
 * Print the menu to the serial interface
 */
void SerialConsole::printMenu()
{
    //Show build # here as well in case people are using the native port and don't get to see the start up messages
    Logger::console(F("\n%s"), CFG_VERSION);
    Logger::console(F("System State: %s"), status.systemStateToStr(status.getSystemState()).c_str());
    Logger::console(F("System Menu:\n"));
    Logger::console(F("Enable line endings of some sort (LF, CR, CRLF)\n"));
    Logger::console(F("Commands:"));
    Logger::console(F("h = help (displays this message)"));
    Logger::console(F("r = reset configuration"));
    Logger::console(F("s = save configuration to EEPROM"));
    Logger::console(F("l = load configuration from EEPROM"));
    Logger::console(F("x = stop program"));
    Logger::console(F("start=<program #> - start program number"));

    Logger::console(F("\nConfig Commands (enter command=newvalue)\n"));
    Logger::console(F("LOGLEVEL=%d - set log level (0=debug, 1=info, 2=warn, 3=error, 4=off)"), Logger::getLogLevel());

    printMenuParams();
    printMenuSensors();
    printMenuIO();
    printMenuProgram();
}

void SerialConsole::printMenuParams()
{
    ConfigurationParams* configParams = Configuration::getParams();
    Logger::console(F("NUM_PLATES=%d - number of installed plates (0-15, default: 4)"), configParams->numberOfPlates);
    Logger::console(F("HIVE_OT=%d - hive over-temp (in 0.1 deg C, default: 460)"), configParams->hiveOverTemp);
    Logger::console(F("HIVE_OTR=%d - hive over-temp recover (in 0.1 deg C, default: 350)"), configParams->hiveOverTempRecover);
    Logger::console(F("PLATE_OT=%d - plate over-temp (in 0.1 deg C, default: 850)"), configParams->plateOverTemp);
    Logger::console(F("MAX_HEAT_CC=%d - max number of concurrent active heaters if PWM disabled (default: 2)"), configParams->maxConcurrentHeaters);
    Logger::console(F("MAX_HEAT_PWR=%d - maximum heater power in PWM mode (0-255, default: 170)"), configParams->maxHeaterPower);
    Logger::console(F("MIN_FAN_SPEED=%d - minimum fan speed level (0-255, default: 10)"), configParams->minFanSpeed);
    Logger::console(F("PWM=%d - enable/disable PWM (0=off, 1=on, default: 0)"), configParams->usePWM);
    Logger::console(F("HUMID_DRY=%d - extended run time to allow humidifier fan to dry (0-255 min, default: 2)"), configParams->humidifierFanDryTime);
}

void SerialConsole::printMenuSensors()
{
    ConfigurationSensor* configSensor = Configuration::getSensor();
    for (int i = 0; configSensor->addressHive[i].value != 0 && i < CFG_MAX_NUMBER_PLATES; i++) {
        Logger::console(F("ADDR_HIVE[%d]=%#08lx%08lx - address of the hive temperature sensor"), i + 1, configSensor->addressHive[i].high,
                configSensor->addressHive[i].low);
    }
    if (configSensor->addressHive[0].value == 0) {
        Logger::console(F("ADDR_HIVE[0]=0x0000000000000000 - address of the hive temperature sensor"));
    }
    for (int i = 0; i < Configuration::getParams()->numberOfPlates; i++) {
        Logger::console(F("ADDR_PLATE[%d]=%#08lx%08lx - address of the plate temperature sensor"), i + 1, configSensor->addressPlate[i].high,
                configSensor->addressPlate[i].low);
    }
}

void SerialConsole::printMenuIO()
{
    ConfigurationIO* configIO = Configuration::getIO();
    Logger::console(F("PIN_BEEP=%d - output pin for beeper (default: 10)"), configIO->beeper);
    Logger::console(F("PIN_NEXT=%d - input pin for button next (default: 55 = A1)"), configIO->buttonNext);
    Logger::console(F("PIN_SELECT=%d - input pin for button select (default: 56 = A2)"), configIO->buttonSelect);
    for (int i = 0; i < Configuration::getParams()->numberOfPlates; i++) {
        Logger::console(F("PIN_FAN[%d]=%d - output pin for fan (default: 7, 8, 44, 45)"), i + 1, configIO->fan[i]);
    }
    for (int i = 0; i < Configuration::getParams()->numberOfPlates; i++) {
        Logger::console(F("PIN_HEATER[%d]=%d - output for for heater (default: 11, 12, 2, 3)"), i + 1, configIO->heater[i]);
    }
    Logger::console(F("PIN_HB=%d - output pin for heartbeat signal (default: 13)"), configIO->heartbeat);
    Logger::console(F("PIN_RELAY=%d - output pin for heater main relay (default: 54 = A0)"), configIO->heaterRelay);
    Logger::console(F("PIN_FAN_HUMID=%d - output pin for humidifier fan (default: 6)"), configIO->humidifierFan);
    Logger::console(F("PIN_HUMID=%d - input pin for humdity sensor (default: 9)"), configIO->humiditySensor);
    Logger::console(F("HUMID_TYPE=%d - humidity sensor type (11, 21, 22, default: 22)"), configIO->humiditySensorType);
    Logger::console(F("PIN_LCD_D4=%d - output pin LCD D4 (default: 24)"), configIO->lcdD4);
    Logger::console(F("PIN_LCD_D5=%d - output pin LCD D5 (default: 25)"), configIO->lcdD5);
    Logger::console(F("PIN_LCD_D6=%d - output pin LCD D6 (default: 26)"), configIO->lcdD6);
    Logger::console(F("PIN_LCD_D7=%d - output pin LCD D7 (default: 27)"), configIO->lcdD7);
    Logger::console(F("PIN_LCD_EN=%d - output pin LCD enable (default: 23)"), configIO->lcdEnable);
    Logger::console(F("PIN_LCD_RS=%d - output pin LCD RS (default: 22)"), configIO->lcdRs);
    Logger::console(F("PIN_TEMP=%d - input pin temperature sensors (default: 4)"), configIO->temperatureSensor);
    Logger::console(F("PIN_VAPOR=%d - output pin for vaporizer (default: 5)"), configIO->vaporizer);
}

void SerialConsole::printMenuProgram()
{
    Program* program = ProgramHandler::getInstance()->getRunningProgram();
    if (program) {
        Logger::console(F("\nPROGRAM\n"));
        Logger::console(F("TEMP-PREHEAT=%d - pre-heat hive temperature (in 0.1 deg C, 0-600)"), program->temperaturePreHeat);
        Logger::console(F("TEMP=%d - hive temperature (in 0.1 deg C, 0-600)"), program->temperatureHive);
        Logger::console(F("TEMP-PLATE=%d - max plate temperature (in 0.1 deg C, 0-1000)"), program->temperaturePlate);
        Logger::console(F("FANSPEED-PREHEAT=%d - fan speed during pre-heat (0-255)"), program->fanSpeedPreHeat);
        Logger::console(F("FANSPEED=%d - fan speed (0-255)"), program->fanSpeed);
        Logger::console(F("FANSPEED-HUMID=%d - fan speed of humidifier (0-255)"), program->fanSpeedHumidifier);
        Logger::console(F("HUMIDITY-MIN=%d - relative humidity minimum (0-100)"), program->humidityMinimum);
        Logger::console(F("HUMIDITY-MAX=%d - relative humidity maximum (0-100)"), program->humidityMaximum);
        Logger::console(F("DURATION-PREHEAT=%d - duration of pre-heat cycle (in min)"), program->durationPreHeat);
        Logger::console(F("DURATION=%d - duration of program (in min)"), program->duration);
        Logger::console(F("enter the following values multiplied by 100 (e.g. 25 for 0.25) :"));
        Logger::console(F("HIVE-KP=%s - Kp parameter for hive temperature PID"), String(program->hiveKp).c_str());
        Logger::console(F("HIVE-KI=%s - Ki parameter for hive temperature PID"), String(program->hiveKi).c_str());
        Logger::console(F("HIVE-KD=%s - Kd parameter for hive temperature PID"), String(program->hiveKd).c_str());
        Logger::console(F("PLATE-KP=%s - Kp parameter for plate temperature PID"), String(program->plateKp).c_str());
        Logger::console(F("PLATE-KI=%s - Ki parameter for plate temperature PID"), String(program->plateKi).c_str());
        Logger::console(F("PLATE-KD=%s - Kd parameter for plate temperature PID"), String(program->plateKd).c_str());
        Logger::console(F(""));
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
        Logger::console(F("Command needs a value..ie TEMP=420\n"));
        return false;
    }

    int32_t value = strtol((char *) (cmdBuffer + i), NULL, 0);
    command.toUpperCase();

    if (!handleCmdSystem(command, value) && !handleCmdParams(command, value) && !handleCmdSensor(command, (cmdBuffer + i))
            && !handleCmdIO(command, value) && !handleCmdProgram(command, value)) {
        Logger::warn("unknown command: %s", command.c_str());
        return false;
    } else {
        return true;
    }
}

bool SerialConsole::handleCmdSystem(String &command, int32_t value)
{
    if (command == String(F("START"))) {
        if (ProgramHandler::getInstance()->getRunningProgram() == NULL) {
            Logger::info(F("starting program #%d"), value);
            ProgramHandler::getInstance()->start(value);
        } else {
            Logger::console(F("a program is already running"));
        }
    } else if (command == String(F("STATS"))) {
        if (value == 0) {
            Logger::console(F("resetting statistics"));
            Statistics::getInstance()->reset();
            Statistics::getInstance()->save();
        }
    } else {
        return false;
    }
    return true;
}

bool SerialConsole::handleCmdParams(String &command, int32_t value)
{
    ConfigurationParams *configParams = Configuration::getParams();
    if (command == String(F("NUM_PLATES"))) {
        value = constrain(value, 0, 15);
        Logger::console(F("setting number of installed plates to %d"), value);
        configParams->numberOfPlates = value;
    } else if (command == String(F("HIVE_OT"))) {
        value = constrain(value, 0, 700);
        Logger::console(F("setting hive over-temp to %d.%d"), value / 10, value % 10);
        configParams->hiveOverTemp = value;
    } else if (command == String(F("HIVE_OTR"))) {
        value = constrain(value, 0, 700);
        Logger::console(F("setting hive over-temp recover %d.%d"), value / 10, value % 10);
        configParams->hiveOverTempRecover = value;
    } else if (command == String(F("PLATE_OT"))) {
        value = constrain(value, 0, 999);
        Logger::console(F("setting plate over-temp %d.%d"), value / 10, value % 10);
        configParams->plateOverTemp = value;
    } else if (command == String(F("MAX_HEAT_CC"))) {
        value = constrain(value, 0, CFG_MAX_NUMBER_PLATES);
        Logger::console(F("setting max number of concurrent active heaters to %d"), value);
        configParams->maxConcurrentHeaters = value;
    } else if (command == String(F("MAX_HEAT_PWR"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting maximum heater power to %d"), value);
        configParams->maxHeaterPower = value;
    } else if (command == String(F("MIN_FAN_SPEED"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting minimum fan speed level to %d"), value);
        configParams->minFanSpeed = value;
    } else if (command == String(F("PWM"))) {
        value = constrain(value, 0, CFG_MAX_NUMBER_PLATES);
        Logger::console(F("setting PWM to %d"), value);
        configParams->usePWM = value;
    } else if (command == String(F("HUMID_DRY"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting dry time of humidifier fan to %d min"), value);
        configParams->humidifierFanDryTime = value;
    } else if (command == String(F("LOGLEVEL"))) {
        value = constrain(value, 0, 4);
        Logger::console(F("setting loglevel to %d"), value);
        Logger::setLoglevel((Logger::LogLevel) value);
        configParams->loglevel = value;
    } else {
        return false;
    }
    return true;
}

bool SerialConsole::handleCmdSensor(String &command, char *parameter)
{
    ConfigurationSensor *configSensor = Configuration::getSensor();
    if (command.startsWith(String(F("ADDR_HIVE")))) {
        uint8_t index = getIndex(command);
        if (index-- <= CFG_MAX_NUMBER_PLATES) {
            String lowStr = String("0x") + (char *) (parameter + 10);
            configSensor->addressHive[index].low = strtoul(lowStr.c_str(), NULL, 0);
            String highStr = String(parameter).substring(0, 10);
            configSensor->addressHive[index].high = strtoul(highStr.c_str(), NULL, 0);
            Logger::console(F("setting address of hive sensor[%d] to %#08lx%08lx"), index + 1, configSensor->addressHive[index].high,
                    configSensor->addressHive[index].low);
        }
    } else if (command.startsWith(String(F("ADDR_PLATE")))) {
        uint8_t index = getIndex(command);
        if (index-- <= Configuration::getParams()->numberOfPlates) {
            String lowStr = String("0x") + (char *) (parameter + 10);
            configSensor->addressPlate[index].low = strtoul(lowStr.c_str(), NULL, 0);
            String highStr = String(parameter).substring(0, 10);
            configSensor->addressPlate[index].high = strtoul(highStr.c_str(), NULL, 0);
            Logger::console(F("setting address of plate sensor[%d] to %#08lx%08lx"), index + 1, configSensor->addressPlate[index].high,
                    configSensor->addressPlate[index].low);
        }
    } else {
        return false;
    }
    return true;
}

bool SerialConsole::handleCmdIO(String &command, int32_t value)
{
    ConfigurationIO *configIO = Configuration::getIO();
    if (command == String(F("PIN_BEEP"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin for beeper to %d"), value);
        configIO->beeper = value;
    } else if (command == String(F("PIN_NEXT"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting input pin for button next to %d"), value);
        configIO->buttonNext = value;
    } else if (command == String(F("PIN_SELECT"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting input pin for button select to %d"), value);
        configIO->buttonSelect = value;
    } else if (command.startsWith(String(F("PIN_FAN")))) {
        value = constrain(value, 0, 255);
        uint8_t index = getIndex(command);
        if (index <= Configuration::getParams()->numberOfPlates) {
            Logger::console(F("setting output pin for fan[%d] to %d"), index, value);
            configIO->fan[index - 1] = value;
        }
    } else if (command.startsWith(String(F("PIN_HEATER")))) {
        value = constrain(value, 0, 255);
        uint8_t index = getIndex(command);
        if (index <= Configuration::getParams()->numberOfPlates) {
            Logger::console(F("setting output pin for heater[%d] to %d"), index, value);
            configIO->heater[index - 1] = value;
        }
    } else if (command == String(F("PIN_HB"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin for heartbeat signal to %d"), value);
        configIO->heartbeat = value;
    } else if (command == String(F("PIN_RELAY"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin for heater main relay to %d"), value);
        configIO->heaterRelay = value;
    } else if (command == String(F("PIN_FAN_HUMID"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin for humidifier fan to %d"), value);
        configIO->humidifierFan = value;
    } else if (command == String(F("PIN_HUMID"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting input pin for humdity sensor to %d"), value);
        configIO->humiditySensor = value;
    } else if (command == String(F("HUMID_TYPE"))) {
        if (value != 11 && value != 21 && value != 22)
            value = 22;
        Logger::console(F("setting humidity sensor type to %d"), value);
        configIO->humiditySensorType = value;
    } else if (command == String(F("PIN_LCD_D4"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin LCD D4 to %d"), value);
        configIO->lcdD4 = value;
    } else if (command == String(F("PIN_LCD_D5"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin LCD D5 to %d"), value);
        configIO->lcdD5 = value;
    } else if (command == String(F("PIN_LCD_D6"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin LCD D6 to %d"), value);
        configIO->lcdD6 = value;
    } else if (command == String(F("PIN_LCD_D7"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin LCD D7 to %d"), value);
        configIO->lcdD7 = value;
    } else if (command == String(F("PIN_LCD_EN"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin LCD enable to %d"), value);
        configIO->lcdEnable = value;
    } else if (command == String(F("PIN_LCD_RS"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin LCD RS to %d"), value);
        configIO->lcdRs = value;
    } else if (command == String(F("PIN_TEMP"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting input pin temperature sensors to %d"), value);
        configIO->temperatureSensor = value;
    } else if (command == String(F("PIN_VAPOR"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("setting output pin for vaporizer to %d"), value);
        configIO->vaporizer = value;
    } else {
        return false;
    }
    return true;
}

bool SerialConsole::handleCmdProgram(String &command, int32_t value)
{
    Program *program = ProgramHandler::getInstance()->getRunningProgram();
    if (!program) {
        return false;
    }
    if (command == String(F("TEMP-PREHEAT"))) {
        value = constrain(value, 0, 600);
        Logger::console(F("Setting pre-heat hive temperature to %d.%d deg C"), value / 10, value % 10);
        program->temperaturePreHeat = value;
    } else if (command == String(F("TEMP"))) {
        value = constrain(value, 0, 600);
        Logger::console(F("Setting hive temperature to %d.%d deg C"), value / 10, value % 10);
        program->temperatureHive = value;
    } else if (command == String(F("TEMP-PLATE"))) {
        value = constrain(value, 0, 1000);
        Logger::console(F("Setting max. plate temperature to %d.%d deg C"), value / 10, value % 10);
        program->temperaturePlate = value;
    } else if (command == String(F("FANSPEED-PREHEAT"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("Setting pre-heat fan speed to %d"), value);
        program->fanSpeedPreHeat = value;
        program->changed = true;
    } else if (command == String(F("FANSPEED"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("Setting fan speed to %d"), value);
        program->fanSpeed = value;
        program->changed = true;
    } else if (command == String(F("FANSPEED-HUMID"))) {
        value = constrain(value, 0, 255);
        Logger::console(F("Setting speed of humidifier fan to %d"), value);
        program->fanSpeedHumidifier = value;
        program->changed = true;
    } else if (command == String(F("HUMIDITY-MIN"))) {
        value = constrain(value, 0, 100);
        Logger::console(F("Setting relative humidity minimum to %d %%"), value);
        program->humidityMinimum = value;
        program->changed = true;
    } else if (command == String(F("HUMIDITY-MAX"))) {
        value = constrain(value, 0, 100);
        Logger::console(F("Setting relative humidity maximum to %d %%"), value);
        program->humidityMaximum = value;
        program->changed = true;
    } else if (command == String(F("DURATION-PREHEAT"))) {
        value = constrain(value, 0, 0xffff);
        Logger::console(F("Setting duration of pre-heat cycle to %d min"), value);
        program->durationPreHeat = value;
    } else if (command == String(F("DURATION"))) {
        value = constrain(value, 0, 0xffff);
        Logger::console(F("Setting duration of program to %d min"), value);
        program->duration = value;
    } else if (command == String(F("HIVE-KP"))) {
        program->hiveKp = (double) value / (double) 100.0;
        Logger::console(F("Setting hive temperature Kp to %s"), String(program->hiveKp).c_str());
        program->changed = true;
    } else if (command == String(F("HIVE-KI"))) {
        program->hiveKi = (double) value / (double) 100.0;
        Logger::console(F("Setting hive temperature Ki to %s"), String(program->hiveKi).c_str());
        program->changed = true;
    } else if (command == String(F("HIVE-KD"))) {
        program->hiveKd = (double) value / (double) 100.0;
        Logger::console(F("Setting hive temperature Kd to %s"), String(program->hiveKd).c_str());
        program->changed = true;
    } else if (command == String(F("PLATE-KP"))) {
        program->plateKp = (double) value / (double) 100.0;
        Logger::console(F("Setting plate temperature Kp to %s"), String(program->plateKp).c_str());
        program->changed = true;
    } else if (command == String(F("PLATE-KI"))) {
        program->plateKi = (double) value / (double) 100.0;
        Logger::console(F("Setting plate temperature Ki to %s"), String(program->plateKi).c_str());
        program->changed = true;
    } else if (command == String(F("PLATE-KD"))) {
        program->plateKd = (double) value / (double) 100.0;
        Logger::console(F("Setting plate temperature Kd to %s"), String(program->plateKd).c_str());
        program->changed = true;
    } else {
        return false;
    }
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
        Logger::info(F("Stopping program"));
        ProgramHandler::getInstance()->stop();
        break;

    case 'r':
        Configuration::getInstance()->reset();
        break;

    case 's':
        Configuration::getInstance()->save();
        break;

    case 'l':
        Configuration::getInstance()->load();
        break;

    default:
        return false;
        break;
    }
    return true;
}

/**
 * Extract the index of an parameter with array notation
 */
uint8_t SerialConsole::getIndex(String command)
{
    String indexStr = command.substring(command.indexOf('[') + 1, command.indexOf(']'));
    if (indexStr.length() > 0) {
        return indexStr.toInt();
    }
    return 255;
}
