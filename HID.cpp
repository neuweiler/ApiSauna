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
    itrMenu = NULL;
    itrSubMenu = NULL;
}

void HID::initialize()
{
    Logger::info(F("initializing HID"));
    Device::initialize();
    ConfigurationIO *io = Configuration::getIO();
    lcd.init(1, io->lcdRs, 255, io->lcdEnable, io->lcdD0, io->lcdD1, io->lcdD2, io->lcdD3, 0, 0, 0, 0);
    lcd.begin(20, 4);

    pinMode(io->buttonUp, INPUT);
    pinMode(io->buttonDown, INPUT);
    pinMode(io->buttonLeft, INPUT);
    pinMode(io->buttonRight, INPUT);
    pinMode(io->buttonSelect, INPUT);

    createMenu();
    handleMenu(NONE);
}

void HID::createMenu()
{
    MenuEntry programMenu;
    programMenu.name = F("Select Program:");
    SimpleList<Program> *programs = ProgramHandler::getInstance()->getPrograms();
    for (SimpleList<Program>::iterator itr = programs->begin(); itr != programs->end(); ++itr) {
        SubMenuEntry subMenuEntry;
        subMenuEntry.name = itr->name;
        subMenuEntry.action = START_PROGRAM;
        programMenu.subMenuEntries.push_back(subMenuEntry);
    }
    menuEntries.push_back(programMenu);

    MenuEntry monitorMenu;
    monitorMenu.name = F("Monitor:");
    SubMenuEntry hiveSubMenuEntry;
    hiveSubMenuEntry.name = F("Hive");
    monitorMenu.subMenuEntries.push_back(hiveSubMenuEntry);
    menuEntries.push_back(monitorMenu);

    MenuEntry configMenu;
    configMenu.name = F("Configuration:");
    SubMenuEntry configTest1SubMenuEntry;
    configTest1SubMenuEntry.name = F("Test1");
    monitorMenu.subMenuEntries.push_back(configTest1SubMenuEntry);
    SubMenuEntry configTest2SubMenuEntry;
    configTest2SubMenuEntry.name = F("Test2");
    monitorMenu.subMenuEntries.push_back(configTest2SubMenuEntry);
    SubMenuEntry configTest3SubMenuEntry;
    configTest3SubMenuEntry.name = F("Test3");
    monitorMenu.subMenuEntries.push_back(configTest3SubMenuEntry);
    menuEntries.push_back(configMenu);

    // init the iterators to display the first entry
    itrMenu = menuEntries.begin();
    itrSubMenu = itrMenu->subMenuEntries.begin();
}

void HID::handleMenu(Button button)
{
    switch (button) {
    case UP:
        if (itrSubMenu != itrMenu->subMenuEntries.begin()) {
            itrSubMenu--;
        }
        break;
    case DOWN:
        if (itrSubMenu + 1 != itrMenu->subMenuEntries.end()) {
            itrSubMenu++;
        }
        break;
    case LEFT:
        if (itrMenu != menuEntries.begin()) {
            itrMenu--;
            itrSubMenu = itrMenu->subMenuEntries.begin();
        }
        break;
    case RIGHT:
        if (itrMenu + 1 != menuEntries.end()) {
            itrMenu++;
            itrSubMenu = itrMenu->subMenuEntries.begin();
        }
        break;
    case SELECT:
        selectMenu(itrSubMenu->action);
        break;
    case NONE:
        break;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(CFG_VERSION);
    lcd.setCursor(0, 2);
    lcd.print(itrMenu->name);
    lcd.setCursor(1, 3);
    lcd.print(itrSubMenu->name);
}

void HID::selectMenu(Action action)
{
    switch (action) {
    case START_PROGRAM: {
        int i = 1;
        for (SimpleList<SubMenuEntry>::iterator itr = itrMenu->subMenuEntries.begin(); itr != itrMenu->subMenuEntries.end(); ++itr && ++i) {
            if (itrSubMenu == itr) {
                lcd.clear();
                ProgramHandler::getInstance()->start(i);
                break;
            }
        }
        break;
    }
    case MONITOR_HIVE:
        //TODO handle monitoring
        Logger::info(F("Monitor hive..."));
        break;
    case CONFIG_TEST1:
        //TODO handle configuration
        Logger::info(F("Config test1..."));
        break;
    case CONFIG_TEST2:
        //TODO handle configuration
        Logger::info(F("Config test2..."));
        break;
    case CONFIG_TEST3:
        //TODO handle configuration
        Logger::info(F("Config test3..."));
        break;
    }
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
            handleMenu(button);
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
        for (int i = 0; (Configuration::getSensor()->addressHive[i].value != 0) && (i < CFG_MAX_NUMBER_PLATES); i++) {
            snprintf(lcdBuffer, 21, "%02d\xdf", status->temperatureHive[i] / 10);
            lcd.print(lcdBuffer);
        }
        logData();
        break;
    case Status::shutdown:
        if (lastSystemState != state) {
            lcd.clear();
            lcd.print(F("Program finished"));
        }
        break;
    case Status::error:
        if (lastSystemState != state) {
            lcd.clear();
            snprintf(lcdBuffer, 21, "ERROR: %03d", status->errorCode);
            lcd.print(lcdBuffer);
            lcd.setCursor(0, 1);
            lcd.print(status->errorCodeToStr(status->errorCode));
        }
        logData();
        break;
    }
    lastSystemState = state;
    tickCounter++;
}

/**
 * Display the various program informations in different cycles on the LCD.
 */
void HID::displayProgramInfo()
{
    if (tickCounter > 9) {
        Status *status = Status::getInstance();
        ProgramHandler *programHandler = ProgramHandler::getInstance();

        // program name and time running
        lcd.setCursor(0, 0);
        snprintf(lcdBuffer, 21, "%s", (status->getSystemState() == Status::preHeat ? "pre-heating" : programHandler->getRunningProgram()->name));
        lcd.print(lcdBuffer);
        lcd.setCursor(13, 0);
        snprintf(lcdBuffer, 21, "%s", convertTime(programHandler->getTimeRunning()).c_str());
        lcd.print(lcdBuffer);

        // actual+target hive temperature and humidity with humidifier/fan status
        lcd.setCursor(0, 1);
        for (int i = 0; (Configuration::getSensor()->addressHive[i].value != 0) && (i < 4); i++) {
            snprintf(lcdBuffer, 21, "%02d\xdf", status->temperatureHive[i] / 10);
            lcd.print(lcdBuffer);
        }
        snprintf(lcdBuffer, 21, "\x7e%02d\xdf%s%02d%%  ", status->temperatureTargetHive / 10,
                (status->vaporizerEnabled ? "*" : status->fanSpeedHumidifier > 0 ? "x" : " "), status->humidity);
        lcd.print(lcdBuffer);

        // actual and target plate temperatures
        lcd.setCursor(0, 2);
        for (int i = 0; (i < Configuration::getParams()->numberOfPlates) && (i < 4); i++) {
            snprintf(lcdBuffer, 21, "%02d%c", status->temperaturePlate[i] / 10, (status->powerPlate[i] > 0 ? 0xeb : 0xdf));
            lcd.print(lcdBuffer);
        }
        snprintf(lcdBuffer, 21, "\x7e%02d\xdf %02d\xdf", status->temperatureTargetPlate / 10, status->temperatureHumidifier / 10);
        lcd.print(lcdBuffer);

        // fan speed, time remaining
        lcd.setCursor(0, 3);
        for (int i = 0; i < Configuration::getParams()->numberOfPlates && i < 4; i++) {
            snprintf(lcdBuffer, 21, "%02ld ", map(status->fanSpeedPlate[i], Configuration::getParams()->minFanSpeed, 255, 0, 99));
            lcd.print(lcdBuffer);
        }
        lcd.setCursor(12, 3);
        snprintf(lcdBuffer, 21, " %s", convertTime(programHandler->getTimeRemaining()).c_str());
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

    Logger::info(F("time: %s, remaining: %s, status: %s"), convertTime(programHandler->getTimeRunning()).c_str(),
            convertTime(programHandler->getTimeRemaining()).c_str(), status->systemStateToStr(status->getSystemState()).c_str());

    uint16_t maxTemp = 0;
    for (int i = 0; (Configuration::getSensor()->addressHive[i].value != 0) && (i < CFG_MAX_NUMBER_PLATES); i++) {
        Logger::debug(F("sensor %d: %s C"), i + 1, toDecimal(status->temperatureHive[i], 10).c_str());
        maxTemp = max(maxTemp, status->temperatureHive[i]);
    }
    Logger::info(F("hive: %sC -> %sC"), toDecimal(maxTemp, 10).c_str(), toDecimal(status->temperatureTargetHive, 10).c_str());

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

    if (digitalRead(io->buttonUp))
        return UP;
    if (digitalRead(io->buttonDown))
        return DOWN;
    if (digitalRead(io->buttonLeft))
        return LEFT;
    if (digitalRead(io->buttonRight))
        return RIGHT;
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
