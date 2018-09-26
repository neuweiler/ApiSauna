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
    tickCounter = 0;
    lastButtons = 0;
    resetStamp = 0;
    statusLed = false;
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

    beeper.initialize();

    selectedProgram = ProgramHandler::getInstance()->getPrograms()->begin();
}

void HID::handleProgramMenu()
{
    uint8_t buttons = readButtons();
    if (lastButtons == buttons) {
        return;
    }
    lastButtons = buttons;

    SimpleList<Program> *programs = ProgramHandler::getInstance()->getPrograms();

    if (buttons & NEXT) {
        beeper.click();
        if (selectedProgram + 1 != programs->end()) {
            selectedProgram++;
        } else {
            selectedProgram = programs->begin();
        }
        displayProgramMenu();
    }
    if (buttons & SELECT) {
        beeper.click();
        int i = 1;
        for (SimpleList<Program>::iterator itr = programs->begin(); itr != programs->end(); ++itr && ++i) {
            if (itr == selectedProgram) {
                lcd.clear();
                ProgramHandler::getInstance()->start(i);
                break;
            }
        }
    }
}

void HID::displayProgramMenu()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(CFG_VERSION);
    lcd.setCursor(0, 1);
    lcd.print(F("Select Program:"));
    lcd.setCursor(1, 2);
    lcd.print(selectedProgram->name);
    lcd.setCursor(0, 3);
    lcd.print(F("next           start"));
}

bool HID::modal(String request, String negative, String positive, uint8_t timeout = 5)
{
    ProgramHandler::getInstance()->pause(); // make sure the heater's are off during the modal

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(request);
    lcd.setCursor(0, 3);
    lcd.print(negative);
    lcd.setCursor(20 - positive.length(), 3);
    lcd.print(positive);

    // wait for button release or timeout
    uint16_t countdown = timeout * 10;
    while (countdown-- > 0 && readButtons() != 0) {
        delay(100);
    }
    // wait for button press or timeout
    while (countdown-- > 0 && readButtons() == 0) {
        delay(100);
    }
    lcd.clear();
    beeper.click();

    ProgramHandler::getInstance()->resume();
    return readButtons() & SELECT;
}

void HID::handleProgramInput()
{
    uint8_t buttons = readButtons();
    if (lastButtons == buttons) {
        return;
    }
    lastButtons = buttons;

    Status::SystemState state = status.getSystemState();
    if (buttons & NEXT) {
        beeper.click();
        if (state == Status::preHeat) {
            if (modal(F("Skip pre-heating?"), F("no"), F("yes"))) {
                ProgramHandler::getInstance()->switchToRunning();
            }
        }
        if (state == Status::running) {
            if (modal(F("Abort program?"), F("no"), F("yes"))) {
                ProgramHandler::getInstance()->stop();
            }
        }
    }
}

void HID::handleFinishedInput()
{
    uint8_t buttons = readButtons();
    if (lastButtons == buttons) {
        return;
    }
    lastButtons = buttons;

    if (buttons & NEXT) {
        beeper.click();
        if (modal(F("Extend program?"), F("no"), F("yes"))) {
            ProgramHandler::getInstance()->addTime(30);
        } else {
            displayFinishedMenu();
        }
    }
    if (buttons & SELECT) {
        beeper.click();
        status.setSystemState(Status::ready);
    }
}


void HID::checkReset()
{
    uint8_t buttons = readButtons();
    if ((buttons & NEXT) && (buttons & SELECT)) {
        if (resetStamp == 0) {
            resetStamp = millis();
        } else if (millis() - resetStamp > 3000) {
            softReset();
        } else {
            beeper.click();
        }
    } else {
        resetStamp = 0;
    }
}

void HID::displayFinishedMenu()
{
    lcd.print(F("Program finished"));
    lcd.setCursor(0, 3);
    lcd.print(F("+30min          menu"));
}

void HID::stateSwitch(Status::SystemState fromState, Status::SystemState toState)
{
    lcd.clear();
    switch (toState) {
    case Status::ready:
        displayProgramMenu();
        beeper.beep(2);
        break;
    case Status::overtemp:
        lcd.print(F("OVER-TEMPERATURE !!!"));
        beeper.beep(-1);
        break;
    case Status::shutdown:
        displayFinishedMenu();
        break;
    case Status::error:
        beeper.beep(20);
        snprintf(lcdBuffer, 21, "ERROR: %03d", status.errorCode);
        lcd.print(lcdBuffer);
        lcd.setCursor(0, 1);
        lcd.print(status.getError());
        break;
    default:
        beeper.beep(2);
    }
}

void HID::process()
{
    Device::process();
    checkReset();

    Status::SystemState state = status.getSystemState();

    if (state != lastSystemState) {
        stateSwitch(lastSystemState, state);
        lastSystemState = state;
    }

    switch (state) {
    case Status::init:
        break;
    case Status::ready:
        handleProgramMenu();
        break;
    case Status::preHeat:
    case Status::running:
        displayProgramInfo();
        handleProgramInput();
        break;
    case Status::overtemp:
        displayHiveTemperatures(1, true);
        break;
    case Status::shutdown:
        displayHiveTemperatures(1, true);
        handleFinishedInput();
        break;
    case Status::error:
        displayHiveTemperatures(3, true);
        break;
    }

    beeper.process();
    logData();

    statusLed = !statusLed;
    digitalWrite(Configuration::getIO()->heartbeat, statusLed); // some kind of heart-beat

    if (tickCounter++ > 8)
        tickCounter = 0;
}

void HID::displayHiveTemperatures(uint8_t row, bool displayAll)
{
    if (tickCounter != 0)
        return;

    lcd.setCursor(0, row);
    for (int i = 0; i < (displayAll ? CFG_MAX_NUMBER_PLATES : 4); i++) {
        if (Configuration::getSensor()->addressHive[i].value != 0) {
            snprintf(lcdBuffer, 4, "%02d\xdf", (status.temperatureHive[i] + 5) / 10);
            lcd.print(lcdBuffer);
        }
    }
}

/**
 * Display the various program informations in different cycles on the LCD.
 */
void HID::displayProgramInfo()
{
    if (tickCounter != 0)
        return;

    ProgramHandler *programHandler = ProgramHandler::getInstance();

    // program name and time running
    lcd.setCursor(0, 0);
    snprintf(lcdBuffer, 14, "%-13s", (status.getSystemState() == Status::preHeat ? "pre-heating" : programHandler->getRunningProgram()->name));
    lcd.print(lcdBuffer);
    String timeRunning = convertTime(programHandler->calculateTimeRunning());
    lcd.setCursor(timeRunning.length() == 7 ? 13 : 12, 0);
    snprintf(lcdBuffer, 9, "%s", timeRunning.c_str());
    lcd.print(lcdBuffer);

    // actual+target hive temperature and humidity with humidifier/fan status
    displayHiveTemperatures(1, false);
    snprintf(lcdBuffer, 21, "\x7e%02d\xdf%s%02d%%  ", (status.temperatureTargetHive + 5) / 10,
            (status.vaporizerEnabled ? "*" : status.fanSpeedHumidifier > 0 ? "x" : " "), status.humidity);
    lcd.print(lcdBuffer);

    // actual and target plate temperatures
    lcd.setCursor(0, 2);
    for (int i = 0; (i < Configuration::getParams()->numberOfPlates) && (i < 4); i++) {
        snprintf(lcdBuffer, 4, "%02d%c", (status.temperaturePlate[i] + 5) / 10, (status.powerPlate[i] > 0 ? 0xeb : 0xdf));
        lcd.print(lcdBuffer);
    }
    snprintf(lcdBuffer, 9, "\x7e%02d\xdf %02d\xdf", (status.temperatureTargetPlate + 5) / 10, (status.temperatureHumidifier + 5) / 10);
    lcd.print(lcdBuffer);

    // fan speed, time remaining
    lcd.setCursor(0, 3);
    for (int i = 0; i < Configuration::getParams()->numberOfPlates && i < 4; i++) {
        snprintf(lcdBuffer, 4, "%02ld ", map(status.fanSpeedPlate[i], Configuration::getParams()->minFanSpeed, 255, 0, 99));
        lcd.print(lcdBuffer);
    }
    String timeRemaining = convertTime(programHandler->calculateTimeRemaining());
    lcd.setCursor(timeRemaining.length() == 7 ? 12 : 11, 3);
    snprintf(lcdBuffer, 10, " %s", timeRemaining.c_str());
    lcd.print(lcdBuffer);
}

/**
 * \brief Print the current data with the logger.
 *
 */
void HID::logData()
{
    if (tickCounter != 3)
        return;

    ProgramHandler *programHandler = ProgramHandler::getInstance();

    Logger::info(F("time: %s, remaining: %s, status: %s"), convertTime(programHandler->calculateTimeRunning()).c_str(),
            convertTime(programHandler->calculateTimeRemaining()).c_str(), status.systemStateToStr(status.getSystemState()).c_str());

    for (int i = 0; (Configuration::getSensor()->addressHive[i].value != 0) && (i < CFG_MAX_NUMBER_PLATES); i++) {
        Logger::debug(F("sensor %d: %s C"), i + 1, toDecimal(status.temperatureHive[i], 10).c_str());
    }
    Logger::info(F("hive: %sC -> %sC"), toDecimal(status.temperatureActualHive, 10).c_str(), toDecimal(status.temperatureTargetHive, 10).c_str());

    for (int i = 0; i < Configuration::getParams()->numberOfPlates; i++) {
        Logger::info(F("plate %d: %sC -> %sC, power=%d/%d, fan=%d"), i + 1, toDecimal(status.temperaturePlate[i], 10).c_str(),
                toDecimal(status.temperatureTargetPlate, 10).c_str(), status.powerPlate[i], Configuration::getParams()->maxHeaterPower,
                status.fanSpeedPlate[i]);
    }

    Logger::info(F("humidity: relHumidity=%d (%d-%d), vapor=%d, fan=%d, temp=%s C"), status.humidity,
            programHandler->getRunningProgram()->humidityMinimum, programHandler->getRunningProgram()->humidityMaximum, status.vaporizerEnabled,
            status.fanSpeedHumidifier, toDecimal(status.temperatureHumidifier, 10).c_str());
}

/**
 * \brief Read which buttons are pressed.
 *
 * \return the pressed button in a bitmask
 */
uint8_t HID::readButtons()
{
    ConfigurationIO *io = Configuration::getIO();
    uint8_t buttons = 0;
    buttons |= digitalRead(io->buttonNext) ? NEXT : 0;
    buttons |= digitalRead(io->buttonSelect) ? SELECT : 0;
    return buttons;
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

void HID::softReset()
{
    asm volatile ("  jmp 0");
}
