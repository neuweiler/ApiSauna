/*
 * HID.cpp
 *
 * Controls the output to a display (LCD screen) and reacts on human input
 * to control the sauna.
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

#include "HID.h"

HID::HID() :
        Device()
{
    lastSystemState = Status::init;
    lastSelectedButton = NONE;
    tickCounter = 0;
    selectedProgram = NULL;
}

void HID::initialize()
{
    Logger::info(F("initializing HID"));
    Device::initialize();
    ConfigurationIO *io = Configuration::getIO();
    lcd.init(1, io->lcdRs, 255, io->lcdEnable, io->lcdD0, io->lcdD1, io->lcdD2, io->lcdD3, 0, 0, 0, 0);
    lcd.begin(20, 4);

    pinMode(io->buttonNext, INPUT);
    pinMode(io->buttonSelect, INPUT);

    selectedProgram = ProgramHandler::getInstance()->getPrograms()->begin();
    handleInput(NONE);
}

void HID::handleInput(Button button)
{
    int i = 1;
    SimpleList<Program> *programs = ProgramHandler::getInstance()->getPrograms();

    switch (button) {
    case NEXT:
        if (selectedProgram + 1 != programs->end()) {
            selectedProgram++;
        } else {
            selectedProgram = programs->begin();
        }
        break;
    case SELECT:
        for (SimpleList<Program>::iterator itr = programs->begin(); itr != programs->end(); ++itr && ++i) {
            if (itr == selectedProgram) {
                lcd.clear();
                ProgramHandler::getInstance()->start(i);
                break;
            }
        }
        break;
    case NONE:
        break;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(CFG_VERSION);
    lcd.setCursor(0, 2);
    lcd.print(F("Select Program:"));
    lcd.setCursor(1, 3);
    lcd.print(selectedProgram->name);
}

void HID::process()
{
    Device::process();

    Button button = buttonPressed();
    lcd.setCursor(0, 0);
    Status *status = Status::getInstance();
    Status::SystemState state = status->getSystemState();
    switch (state) {
    case Status::init:
        break;
    case Status::ready: // allow user to navigate through menu
        if (lastSelectedButton != button) {
            handleInput(button);
            lastSelectedButton = button;
        }
        break;
    case Status::preHeat:
    case Status::running:
        displayProgramInfo();
        logData();
        break;
    case Status::overtemp:
        lcd.clear();
        lcd.print(F("OVER-TEMPERATURE !!!"));
        lcd.setCursor(0, 1);
        displayHiveTemperatures(true);
        logData();
        break;
    case Status::shutdown:
        lcd.clear();
        lcd.print(F("Program finished"));
        lcd.setCursor(0, 2);
        displayHiveTemperatures(true);
        logData();
        if (button == SELECT) {
            status->setSystemState(Status::ready);
            handleInput(NONE);
            lastSelectedButton = button;
        }
        break;
    case Status::error:
        lcd.clear();
        snprintf(lcdBuffer, 21, "ERROR: %03d", status->errorCode);
        lcd.print(lcdBuffer);
        lcd.setCursor(0, 1);
        lcd.print(status->errorCodeToStr(status->errorCode));
        lcd.setCursor(0, 3);
        displayHiveTemperatures(true);
        logData();
        break;
    }
    lastSystemState = state;
    tickCounter++;
}

void HID::displayHiveTemperatures(bool displayAll)
{
    Status *status = Status::getInstance();
    for (int i = 0; i < (displayAll ? CFG_MAX_NUMBER_PLATES : 4); i++) {
        if (Configuration::getSensor()->addressHive[i].value != 0) {
            snprintf(lcdBuffer, 21, "%02d\xdf", (status->temperatureHive[i] + 5) / 10);
            lcd.print(lcdBuffer);
        }
    }
}

/**
 * Display the various program informations in different cycles on the LCD.
 */
void HID::displayProgramInfo()
{
    if (tickCounter > 7) {
        Status *status = Status::getInstance();
        ProgramHandler *programHandler = ProgramHandler::getInstance();

        // program name and time running
        lcd.setCursor(0, 0);
        snprintf(lcdBuffer, 21, "%s", (status->getSystemState() == Status::preHeat ? "pre-heating" : programHandler->getRunningProgram()->name));
        lcd.print(lcdBuffer);
        lcd.setCursor(13, 0);
        snprintf(lcdBuffer, 21, "%s", convertTime(programHandler->calculateTimeRunning()).c_str());
        lcd.print(lcdBuffer);

        // actual+target hive temperature and humidity with humidifier/fan status
        lcd.setCursor(0, 1);
        displayHiveTemperatures(false);
        snprintf(lcdBuffer, 21, "\x7e%02d\xdf%s%02d%%  ", (status->temperatureTargetHive + 5) / 10,
                (status->vaporizerEnabled ? "*" : status->fanSpeedHumidifier > 0 ? "x" : " "), status->humidity);
        lcd.print(lcdBuffer);

        // actual and target plate temperatures
        lcd.setCursor(0, 2);
        for (int i = 0; (i < Configuration::getParams()->numberOfPlates) && (i < 4); i++) {
            snprintf(lcdBuffer, 21, "%02d%c", (status->temperaturePlate[i] + 5) / 10, (status->powerPlate[i] > 0 ? 0xeb : 0xdf));
            lcd.print(lcdBuffer);
        }
        snprintf(lcdBuffer, 21, "\x7e%02d\xdf %02d\xdf", (status->temperatureTargetPlate + 5) / 10, (status->temperatureHumidifier + 5) / 10);
        lcd.print(lcdBuffer);

        // fan speed, time remaining
        lcd.setCursor(0, 3);
        for (int i = 0; i < Configuration::getParams()->numberOfPlates && i < 4; i++) {
            snprintf(lcdBuffer, 21, "%02ld ", map(status->fanSpeedPlate[i], Configuration::getParams()->minFanSpeed, 255, 0, 99));
            lcd.print(lcdBuffer);
        }
        lcd.setCursor(12, 3);
        snprintf(lcdBuffer, 21, " %s", convertTime(programHandler->calculateTimeRemaining()).c_str());
        lcd.print(lcdBuffer);

        tickCounter = 0;
    }
}

/**
 * \brief Print the current data with the logger.
 *
 */
void HID::logData()
{
    if (tickCounter != 0)
        return;

    Status *status = Status::getInstance();
    ProgramHandler *programHandler = ProgramHandler::getInstance();

    Logger::info(F("time: %s, remaining: %s, status: %s"), convertTime(programHandler->calculateTimeRunning()).c_str(),
            convertTime(programHandler->calculateTimeRemaining()).c_str(), status->systemStateToStr(status->getSystemState()).c_str());

    for (int i = 0; (Configuration::getSensor()->addressHive[i].value != 0) && (i < CFG_MAX_NUMBER_PLATES); i++) {
        Logger::debug(F("sensor %d: %s C"), i + 1, toDecimal(status->temperatureHive[i], 10).c_str());
    }
    Logger::info(F("hive: %sC -> %sC"), toDecimal(status->temperatureActualHive, 10).c_str(), toDecimal(status->temperatureTargetHive, 10).c_str());

    for (int i = 0; i < Configuration::getParams()->numberOfPlates; i++) {
        Logger::info(F("plate %d: %sC -> %sC, power=%d/%d, fan=%d"), i + 1, toDecimal(status->temperaturePlate[i], 10).c_str(),
                toDecimal(status->temperatureTargetPlate, 10).c_str(), status->powerPlate[i], Configuration::getParams()->maxHeaterPower,
                status->fanSpeedPlate[i]);
    }

    Logger::info(F("humidity: relHumidity=%d (%d-%d), vapor=%d, fan=%d, temp=%s C"), status->humidity,
            programHandler->getRunningProgram()->humidityMinimum, programHandler->getRunningProgram()->humidityMaximum, status->vaporizerEnabled,
            status->fanSpeedHumidifier, toDecimal(status->temperatureHumidifier, 10).c_str());
}

/**
 * \brief Finds out which button on the LCD display/button combo was pressed.
 *
 * \return the mapped button
 */
HID::Button HID::buttonPressed()
{
    ConfigurationIO *io = Configuration::getIO();

    if (digitalRead(io->buttonNext))
        return NEXT;
    if (digitalRead(io->buttonSelect))
        return SELECT;

    return NONE;
}

/**
 * \brief Convert milliseconds to in hh:mm:ss
 *
 * \return the time in hh:mm:ss
 */
String HID::convertTime(uint32_t seconds)
{
    char buffer[10];
    int8_t hours = (seconds / 3600) % 24;
    int8_t minutes = (seconds / 60) % 60;
    sprintf(buffer, "%d:%02d:%02d", hours, minutes, (int) (seconds % 60));
    return String(buffer);
}

/**
 * Convert integer to decimal
 */
String HID::toDecimal(int16_t number, uint8_t divisor)
{
    char buffer[10];
    snprintf(buffer, 9, "%d.%d", number / divisor, abs(number % divisor));
    return String(buffer);
}
