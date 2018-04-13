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

HID::HID()
{
    lastSystemState = Status::init;
    lastSelectedButton = NONE;
    tickCounter = 0;
    itrMenu = NULL;
    itrSubMenu = NULL;
}

HID::~HID()
{
}

void HID::init()
{
    //TODO use values from config
    lcd.init(1, CFG_IO_LCD_RS, 255, CFG_IO_LCD_ENABLE, CFG_IO_LCD_D0, CFG_IO_LCD_D1, CFG_IO_LCD_D2, CFG_IO_LCD_D3, 0, 0, 0, 0);
    lcd.begin(20, 4);

    pinMode(CFG_IO_BUTTON_UP, INPUT);
    pinMode(CFG_IO_BUTTON_DOWN, INPUT);
    pinMode(CFG_IO_BUTTON_LEFT, INPUT);
    pinMode(CFG_IO_BUTTON_RIGHT, INPUT);
    pinMode(CFG_IO_BUTTON_SELECT, INPUT);

    createMenu();
    handleMenu(NONE);
}

void HID::createMenu()
{
    MenuEntry programMenu;
    programMenu.name = "Select Program:";
    SimpleList<ControllerProgram> *programs = controller.getPrograms();
    for (SimpleList<ControllerProgram>::iterator itr = programs->begin(); itr != programs->end(); ++itr) {
        SubMenuEntry subMenuEntry;
        subMenuEntry.name = itr->name;
        subMenuEntry.action = START_PROGRAM;
        programMenu.subMenuEntries.push_back(subMenuEntry);
    }
    menuEntries.push_back(programMenu);

    MenuEntry monitorMenu;
    monitorMenu.name = "Monitor:";
    SubMenuEntry hiveSubMenuEntry;
    hiveSubMenuEntry.name = "Hive";
    monitorMenu.subMenuEntries.push_back(hiveSubMenuEntry);
    menuEntries.push_back(monitorMenu);

    MenuEntry configMenu;
    configMenu.name = "Configuration:";
    SubMenuEntry configTest1SubMenuEntry;
    configTest1SubMenuEntry.name = "Test1";
    monitorMenu.subMenuEntries.push_back(configTest1SubMenuEntry);
    SubMenuEntry configTest2SubMenuEntry;
    configTest2SubMenuEntry.name = "Test2";
    monitorMenu.subMenuEntries.push_back(configTest2SubMenuEntry);
    SubMenuEntry configTest3SubMenuEntry;
    configTest3SubMenuEntry.name = "Test3";
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
        if (itrSubMenu+1 != itrMenu->subMenuEntries.end()) {
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
        if (itrMenu+1 != menuEntries.end()) {
            itrMenu++;
            itrSubMenu = itrMenu->subMenuEntries.begin();
        }
        break;
    case SELECT: //TODO move this stuff to separate method
        switch (itrSubMenu->action) {
        case START_PROGRAM: {
            int i = 0;

            for (SimpleList<SubMenuEntry>::iterator itr = itrMenu->subMenuEntries.begin(); itr != itrMenu->subMenuEntries.end(); ++itr && ++i) {
                if (itrSubMenu == itr) {
                    lcd.clear();
                    controller.startProgram(i);
                    return;
                }
            }
            break;
        }
        case MONITOR_HIVE:
            //TODO handle monitoring
            Logger::info("Monitor hive...");
            break;
        case CONFIG_TEST1:
            //TODO handle configuration
            Logger::info("Config test1...");
            break;
        case CONFIG_TEST2:
            //TODO handle configuration
            Logger::info("Config test2...");
            break;
        case CONFIG_TEST3:
            //TODO handle configuration
            Logger::info("Config test3...");
            break;
        }
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

void HID::loop()
{
    Button button = buttonPressed();
    lcd.setCursor(0, 0);
    Status::SystemState state = status.getSystemState();
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
        lcd.print("OVER-TEMPERATURE !!!");
        lcd.setCursor(0, 1);
        lcd.print(toDecimal(controller.getHiveTemperature(), 10));
        logData();
        break;
    case Status::shutdown:
        if (lastSystemState != state) {
            lcd.clear();
            lcd.print("Program finished");
        }
        break;
    case Status::error:
        if (lastSystemState != state) {
            lcd.clear();
            lcd.print("ERROR !");
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
        SimpleList<TemperatureSensor> *hiveSensors = controller.getHiveTempSensors();
        Humidifier *humidifier = controller.getHumidifier();
        SimpleList<TemperatureSensor>::iterator itr = hiveSensors->begin();

        // program name and time running
        lcd.setCursor(0, 0);
        snprintf(lcdBuffer, 21, "%s", (status.getSystemState() == Status::preHeat ? "pre-heating" : controller.getRunningProgram()->name));
        lcd.print(lcdBuffer);
        lcd.setCursor(12, 0);
        snprintf(lcdBuffer, 21, " %s", convertTime(controller.getTimeRunning()).c_str());
        lcd.print(lcdBuffer);

        // actual+target hive temperature and humidity with humidifier/fan status
        lcd.setCursor(0, 1);
        while (itr != hiveSensors->end()) {
            snprintf(lcdBuffer, 21, "%02d\xdf", itr->getTemperatureCelsius() / 10);
            lcd.print(lcdBuffer);
            itr++;
        }
        snprintf(lcdBuffer, 21, "\x7e%02d\xdf%s%02d%%  ", controller.getHiveTargetTemperature() / 10,
                (humidifier->getVaporizerMode() ? "*" : humidifier->getFanSpeed() ? "x" : ""), controller.getHumidifier()->getHumidity());
        lcd.print(lcdBuffer);

        // actual and target plate temperatures
        lcd.setCursor(0, 2);
        SimpleList<Plate> *plates = controller.getPlates();
        for (SimpleList<Plate>::iterator itr = plates->begin(); itr != plates->end(); ++itr) {
            snprintf(lcdBuffer, 21, "%02d%c", itr->getTemperature() / 10, (itr->getPower() > 0 ? 0xeb : 0xdf));
            lcd.print(lcdBuffer);
        }
        snprintf(lcdBuffer, 21, "\x7e%02d\xdf %02d\xdf", plates->begin()->getTargetTemperature() / 10, controller.getHumidifier()->getTemperature() / 10);
        lcd.print(lcdBuffer);

        // fan speed, time remaining
        lcd.setCursor(0, 3);
        plates = controller.getPlates();
        for (SimpleList<Plate>::iterator itr = plates->begin(); itr != plates->end(); ++itr) {
            snprintf(lcdBuffer, 21, "%02ld ", map(itr->getFanSpeed(), CFG_MIN_FAN_SPEED, 255, 0, 99));
            lcd.print(lcdBuffer);
        }
        lcd.setCursor(12, 3);
        snprintf(lcdBuffer, 21, " %s", convertTime(controller.getTimeRemaining()).c_str());
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
    Logger::info("time running: %s, remaining: %s, status: %s", convertTime(controller.getTimeRunning()).c_str(),
            convertTime(controller.getTimeRemaining()).c_str(), status.systemStateToStr(status.getSystemState()).c_str());

    SimpleList<TemperatureSensor> *hiveSensors = controller.getHiveTempSensors();
    for (SimpleList<TemperatureSensor>::iterator itr = hiveSensors->begin(); itr != hiveSensors->end(); ++itr) {
        int16_t temp = itr->getTemperatureCelsius();
        Logger::debug("sensor %#lx%lx: %sC", itr->getAddress().high, itr->getAddress().low, toDecimal(temp, 10).c_str());
    }
    Logger::info("hive: %sC -> %sC", toDecimal(controller.getHiveTemperature(), 10).c_str(),
            toDecimal(controller.getHiveTargetTemperature(), 10).c_str());

    SimpleList<Plate> *plates = controller.getPlates();
    for (SimpleList<Plate>::iterator itr = plates->begin(); itr != plates->end(); ++itr) {
        Logger::info("plate %d: %sC -> %sC, power=%d/%d, fan=%d", itr->getId(), toDecimal(itr->getTemperature(), 10).c_str(),
                toDecimal(itr->getTargetTemperature(), 10).c_str(), itr->getPower(), itr->getMaximumPower(), itr->getFanSpeed());
    }

    Humidifier *humidifier = controller.getHumidifier();
    Logger::info("humidity: relHumidity=%d (%d-%d), vapor=%d, fan=%d", humidifier->getHumidity(), humidifier->getMinHumidity(),
            humidifier->getMaxHumidity(), humidifier->getVaporizerMode(), humidifier->getFanSpeed());
}

/**
 * \brief Finds out which button on the LCD display/button combo was pressed.
 *
 * \return the mapped button
 */
HID::Button HID::buttonPressed()
{
    if (digitalRead(CFG_IO_BUTTON_UP))
        return UP;
    if (digitalRead(CFG_IO_BUTTON_DOWN))
        return DOWN;
    if (digitalRead(CFG_IO_BUTTON_LEFT))
        return LEFT;
    if (digitalRead(CFG_IO_BUTTON_RIGHT))
        return RIGHT;
    if (digitalRead(CFG_IO_BUTTON_SELECT))
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
    snprintf(buffer, CFG_LOG_BUFFER_SIZE, "%d.%d", number / divisor, abs(number % divisor));
    return String(buffer);
}

