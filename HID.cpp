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
    baseValue = 1023;
    controlPin = 0;
    menuXPos = 0;
    menuYPos = 0;
    lastSelectedButton = NONE;
    tickCounter = 0;

    menu[0] = "Select Program:";
    subMenu[0][0] = "1 Varroa Killer";
    subMenu[0][1] = "2 Wellness";
    subMenu[0][2] = "3 Queen Health";
    subMenu[0][3] = "4 Melt Honey";
    subMenu[0][4] = "";

    menu[1] = "Monitor:";
    subMenu[1][0] = "1 Hive";
    subMenu[1][1] = "2 gugus";
    subMenu[1][2] = "";

    menu[2] = "Configuration:";
    subMenu[2][0] = "1 - xxxxx";
    subMenu[2][1] = "";

    menu[3] = "Test:";
    subMenu[3][0] = "1 - yyyyy";
    subMenu[3][1] = "";

    menu[4] = "";

}

HID::~HID()
{
}

void HID::init()
{
    //TODO use values from config
    lcd.init(1, CFG_IO_LCD_RS, 255, CFG_IO_LCD_ENABLE, CFG_IO_LCD_D0, CFG_IO_LCD_D1, CFG_IO_LCD_D2, CFG_IO_LCD_D3, 0, 0, 0, 0);
    lcd.begin(16, 2);
    controlPin = CFG_IO_LCD_BUTTON;
    baseValue = 800; //analogRead(controlPin); // adjust to platform's base value
    handleMenu(NONE);
}

void HID::handleMenu(Button button)
{
    switch (button) {
    case UP:
        menuYPos = max(0, menuYPos - 1);
        break;
    case DOWN:
        if (subMenu[menuXPos][menuYPos + 1].length() != 0) {
            // there's a next menu entry
            menuYPos++;
        }
        break;
    case LEFT:
        menuXPos = max(0, menuXPos - 1);
        menuYPos = 0;
        break;
    case RIGHT:
        if (menu[menuXPos + 1].length() != 0) {
            // there's a next menu entry
            menuXPos++;
        }
        menuYPos = 0;
        break;
    case SELECT:
        switch (menuXPos) {
        case 0:
            controller.startProgram(menuYPos);
            break;
        case 1:
            //TODO handle configuration
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        }
        break;
    case NONE:
        break;
    }

    lcd.setCursor(0, 0);
    lcd.print(menu[menuXPos]); // print the selected menu into first row
    lcd.print("     ");
    lcd.setCursor(0, 1);  // print the selected sub-menu into second row
    lcd.print(subMenu[menuXPos][menuYPos]);
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
        if (button != NONE) {
            tickCounter = 0;
        }
        displayProgramInfo();
        logData();
        break;
    case Status::running:
        if (button != NONE) {
            tickCounter = 0;
        }
        displayProgramInfo();
        logData();
        break;
    case Status::overtemp:
        lcd.print("OVER-TEMPERATURE");
        lcd.setCursor(0, 1);
        lcd.print(toDecimal(controller.getHiveTemperature(), 10));
        logData();
        break;
    case Status::shutdown:
        lcd.print("Program finished");
        break;
    case Status::error:
        lcd.print("ERROR !         ");
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
    if (tickCounter < 50) { // hive temp + humidity (2nd line: hive sensors)
        SimpleList<TemperatureSensor> *hiveSensors = controller.getHiveTempSensors();
        Humidifier *humidifier = controller.getHumidifier();
        SimpleList<TemperatureSensor>::iterator itr = hiveSensors->begin();
        snprintf(lcdBuffer, 16, "H:%s>%s %d%%%s ", toDecimal(controller.getHiveTemperature(), 10).c_str(),
                toDecimal(controller.getHiveTargetTemperature(), 10).c_str(), controller.getHumidifier()->getHumidity(),
                (humidifier->getVaporizerMode() ? "*" : humidifier->getFanSpeed() ? "x" : ""));
        lcd.print(lcdBuffer);
        lcd.setCursor(0, 1);
        while (itr != hiveSensors->end()) {
            snprintf(lcdBuffer, 16, "%d ", itr->getTemperatureCelsius() / 10);
            lcd.print(lcdBuffer);
            itr++;
        }
    } else if (tickCounter < 80) { // plate temp + target, power
        SimpleList<Plate> *plates = controller.getPlates();
        for (SimpleList<Plate>::iterator itr = plates->begin(); itr != plates->end(); ++itr) {
            snprintf(lcdBuffer, 16, "%d%s", itr->getTemperature() / 10, (itr != plates->end() ? " " : ""));
            lcd.print(lcdBuffer);
        }
        snprintf(lcdBuffer, 16, ">%s    ", toDecimal(plates->begin()->getMaximumTemperature(), 10).c_str());
        lcd.print(lcdBuffer);
        lcd.setCursor(0, 1);
        for (SimpleList<Plate>::iterator itr = plates->begin(); itr != plates->end(); ++itr) {
            snprintf(lcdBuffer, 16, "%d ", itr->getPower());
            lcd.print(lcdBuffer);
        }
    } else if (tickCounter < 110) { // program name + run time
        lcd.print("program running "); //TODO replace with name of program
        lcd.setCursor(0, 1);
        snprintf(lcdBuffer, 16, "%s %s", convertTime(controller.getTimeRunning()).c_str(), convertTime(controller.getTimeRemaining()).c_str());
        lcd.print(lcdBuffer);
    } else {
        tickCounter = 0;
    }
}

/**
 * \brief Print the current data with the logger.
 *
 */
void HID::logData()
{
    if (tickCounter % 10)
        return;
    Logger::info("time running: %s, time remaining: %s, status: %s", convertTime(controller.getTimeRunning()).c_str(),
            convertTime(controller.getTimeRemaining()).c_str(), status.systemStateToStr(status.getSystemState()).c_str());

    SimpleList<TemperatureSensor> *hiveSensors = controller.getHiveTempSensors();
    for (SimpleList<TemperatureSensor>::iterator itr = hiveSensors->begin(); itr != hiveSensors->end(); ++itr) {
        int16_t temp = itr->getTemperatureCelsius();
        Logger::debug("sensor %#lx%lx: %s C", itr->getAddress().high, itr->getAddress().low, toDecimal(temp, 10).c_str());
    }
    Logger::info("hive temp: %s C, target: %s C", toDecimal(controller.getHiveTemperature(), 10).c_str(),
            toDecimal(controller.getHiveTargetTemperature(), 10).c_str());

    SimpleList<Plate> *plates = controller.getPlates();
    for (SimpleList<Plate>::iterator itr = plates->begin(); itr != plates->end(); ++itr) {
        Logger::info("plate %d: temp=%s C, target=%s C, power=%d (max=%d), speed=%d", itr->getId(), toDecimal(itr->getTemperature(), 10).c_str(),
                toDecimal(itr->getMaximumTemperature(), 10).c_str(), itr->getPower(), itr->getMaximumPower(), itr->getFanSpeed());
    }

    Humidifier *humidifier = controller.getHumidifier();
    Logger::info("humidity: relHumidity=%d (%d-%d), enabled=%d, speed=%d", humidifier->getHumidity(), humidifier->getMinHumidity(),
            humidifier->getMaxHumidity(), humidifier->getVaporizerMode(), humidifier->getFanSpeed());
}

/**
 * \brief Finds out which button on the LCD display/button combo was pressed.
 *
 * \return the mapped button
 */
HID::Button HID::buttonPressed()
{
    uint16_t sensorValue = constrain (analogRead(controlPin), 0, 800);
    uint8_t button = map(sensorValue, 0, baseValue, 0, 5);
    switch (button) {
    case RIGHT:
        return RIGHT;
    case UP:
        return UP;
    case DOWN:
        return DOWN;
    case LEFT:
        return LEFT;
    case SELECT:
        return SELECT;
    }
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

