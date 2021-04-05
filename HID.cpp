/*
 * HID.cpp
 *
 * Controls the output to a display (LCD screen) and reacts on human input
 * to control the sauna.
 *
 Copyright (c) 2017-2021 Michael Neuweiler

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

HID hid;

HID::HID()
{
    tickCounter = 0;
    lastButtons = 0;
    resetStamp = 0;
    statusLed = false;
    selectedProgram = NULL;
    startTime = 0;
    state = UNKNOWN;

    for (int i = 0; i < CFG_MAX_NUMBER_PLATES; i++) {
    	statusZone[i].id = 255;
    	statusPlate[i].id = 255;
    }
}

HID::~HID() {
}

void HID::handleEvent(Event event, ...)
{
    va_list args;
    va_start(args, event);
    switch(event) {
    case PROCESS:
        process();
        break;
    case PROGRAM_UPDATE:
    case PROGRAM_START:
        runningProgram = va_arg(args, Program);
        state = (runningProgram.preHeat ? PREHEAT : RUNNING);
        startTime = millis();
        break;
    case PROGRAM_STOP:
        Logger::info(F("stopping program"));
        state = FINISHED;
        startTime = 0;
        //TODO use own state
        displayFinishedMenu();
        break;
    case PROGRAM_PAUSE:
        Logger::info(F("pausing program"));
        break;
    case PROGRAM_RESUME:
        Logger::info(F("resuming program"));
        break;
    case TEMPERATURE_HIGH:
        Logger::info(F("High hive temperature!! Pausing heater."));
        break;
    case TEMPERATURE_ALERT:
        lcd.print(F("OVER-TEMPERATURE !!!"));
        beeper.beep(-1);
        break;
    case TEMPERATURE_NORMAL:
    	break;
    case STATUS_HUMIDITY:
        statusHumidity = va_arg(args, StatusHumidity);
    	break;
    case STATUS_ZONE: {
        StatusZone tempZone = va_arg(args, StatusZone);
        if (tempZone.id < CFG_MAX_NUMBER_PLATES) {
        	statusZone[tempZone.id] = tempZone;
        }
    }
    	break;
    case STATUS_PLATE: {
		StatusPlate tempPlate = va_arg(args, StatusPlate);
		if (tempPlate.id < CFG_MAX_NUMBER_PLATES) {
			statusPlate[tempPlate.id] = tempPlate;
		}
    }
    	break;
    case ERROR:
    	state = FAULT;
        lcd.print("ERROR:");
        lcd.setCursor(0, 1);
        lcd.print(va_arg(args, String));
        beeper.beep(20);
        break;
    }
    va_end(args);
}

void HID::initialize()
{
    Logger::info(F("initializing HID"));
    ConfigurationIO *io = configuration.getIO();
    lcd.init(1, io->lcdRs, 255, io->lcdEnable, io->lcdD4, io->lcdD5, io->lcdD6, io->lcdD7, 0, 0, 0, 0);
    lcd.begin(20, 4);

    pinMode(io->buttonNext, INPUT);
    pinMode(io->buttonSelect, INPUT);

    selectedProgram = programList.getPrograms()->begin();

    displayProgramMenu();
    beeper.beep(2);

    eventHandler.subscribe(this);
}

void HID::process()
{
    checkReset();

    switch (state) {
    case READY:
        handleProgramMenu();
        break;
    case PREHEAT:
    case RUNNING:
        displayProgramInfo();
        handleProgramInput();
        break;
    case OVERTEMP:
        displayHiveTemperatures(1, true);
        break;
    case FINISHED:
        displayHiveTemperatures(1, true);
        handleFinishedInput();
        break;
    case FAULT:
        displayHiveTemperatures(3, true);
        break;
    }
    logData();

    statusLed = !statusLed;
    digitalWrite(configuration.getIO()->heartbeat, statusLed); // some kind of heart-beat

    if (tickCounter++ > 8)
        tickCounter = 0;
}

void HID::handleProgramMenu()
{
    uint8_t buttons = readButtons();
    if (lastButtons == buttons) {
        return;
    }
    lastButtons = buttons;

    SimpleList<Program> *programs = programList.getPrograms();

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
        lcd.clear();
        eventHandler.publish(PROGRAM_START, selectedProgram);
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
    eventHandler.publish(PROGRAM_PAUSE); // make sure the heater's are off during the modal

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

    eventHandler.publish(PROGRAM_RESUME);
    return readButtons() & SELECT;
}

void HID::handleProgramInput()
{
    uint8_t buttons = readButtons();
    if (lastButtons == buttons) {
        return;
    }
    lastButtons = buttons;

    if (buttons & NEXT) {
        beeper.click();
        if (state == PREHEAT) {
            if (modal(F("Skip pre-heating?"), F("no"), F("yes"))) {
                switchToRunning();
            }
        }
        if (state == RUNNING) {
            if (modal(F("Abort program?"), F("no"), F("yes"))) {
                eventHandler.publish(PROGRAM_STOP);
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
            addTime(30);
        } else {
            displayFinishedMenu();
        }
    }
    if (buttons & SELECT) {
        beeper.click();
        state = READY;
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

void HID::displayHiveTemperatures(uint8_t row, bool displayAll)
{
    if (tickCounter != 0)
        return;

    lcd.setCursor(0, row);
    for (int i = 0; i < (displayAll ? CFG_MAX_NUMBER_PLATES : 4); i++) {
        if (configuration.getSensor()->addressHive[i].value != 0) {
        	//TODO this might not match.. if we have less zones than hive sensors
            snprintf(lcdBuffer, 4, (char *)F("%02d\xdf"), (statusZone[i].temperatureActual + 5) / 10);
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

    // program name and time running
    lcd.setCursor(0, 0);
    snprintf(lcdBuffer, 14, (char *)F("%-13s"), (state == PREHEAT ? (char *) F("pre-heating") : selectedProgram->name));
    lcd.print(lcdBuffer);
    String timeRunning = convertTime(calculateTimeRunning());
    lcd.setCursor(timeRunning.length() == 7 ? 13 : 12, 0);
    snprintf(lcdBuffer, 9, (char *)F("%s"), timeRunning.c_str());
    lcd.print(lcdBuffer);

    // actual+target hive temperature and humidity with humidifier/fan status
    displayHiveTemperatures(1, false);
    lcd.setCursor(12, 1);
//    snprintf(lcdBuffer, 21, "\x7e%02d\xdf%s%02d%%  ", (status.temperatureTargetHive + 5) / 10,
//            (status.vaporizerEnabled ? "*" : status.fanSpeedHumidifier > 0 ? "x" : " "), status.humidity);
//    lcd.print(lcdBuffer);

    // actual and target plate temperatures
    lcd.setCursor(0, 2);
    for (int i = 0; (i < configuration.getParams()->numberOfPlates) && (i < 4); i++) {
        snprintf(lcdBuffer, 4, (char *)F("%02d%c"), (statusPlate[i].temperatureActual + 5) / 10, (statusPlate[i].power > 0 ? 0xeb : 0xdf));
        lcd.print(lcdBuffer);
    }
    lcd.setCursor(12, 2);
    snprintf(lcdBuffer, 9, (char *)F("\x7e%02d\xdf %02d\xdf"), (statusPlate[0].temperatureTarget + 5) / 10, (statusHumidity.temperature + 5) / 10);
    lcd.print(lcdBuffer);

    // fan speed, time remaining
    lcd.setCursor(0, 3);
    for (int i = 0; i < configuration.getParams()->numberOfPlates && i < 4; i++) {
        snprintf(lcdBuffer, 4, (char *)F("%02ld "), map(statusPlate[i].fanSpeed, configuration.getParams()->minFanSpeed, 255, 0, 99));
        lcd.print(lcdBuffer);
    }
    String timeRemaining = convertTime(calculateTimeRemaining());
    lcd.setCursor(timeRemaining.length() == 7 ? 12 : 11, 3);
    snprintf(lcdBuffer, 10, (char *)F(" %s"), timeRemaining.c_str());
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

    Logger::info(F("time: %s, remaining: %s, status: %s"), convertTime(calculateTimeRunning()).c_str(),
            convertTime(calculateTimeRemaining()).c_str(), stateToStr(state).c_str());

    for (int i = 0; (configuration.getSensor()->addressHive[i].value != 0) && (i < CFG_MAX_NUMBER_PLATES); i++) {
        Logger::debug(F("sensor %d: %s C"), i + 1, toDecimal(statusZone[i].temperatureActual, 10).c_str());
    }
    Logger::info(F("hive target: %sC"), toDecimal(statusZone[0].temperatureTarget, 10).c_str());

    for (int i = 0; i < configuration.getParams()->numberOfPlates; i++) {
        Logger::info(F("plate %d: %sC -> %sC, power=%d/%d, fan=%d"), i + 1, toDecimal(statusPlate[i].temperatureActual, 10).c_str(),
                toDecimal(statusPlate[i].temperatureTarget, 10).c_str(), statusPlate[i].power, configuration.getParams()->maxHeaterPower,
                statusPlate[i].fanSpeed);
    }

//    Logger::info(F("humidity: relHumidity=%d (%d-%d), vapor=%d, fan=%d, temp=%s C"), status.humidity,
//            programHandler->getRunningProgram()->humidityMinimum, programHandler->getRunningProgram()->humidityMaximum, status.vaporizerEnabled,
//            status.fanSpeedHumidifier, toDecimal(status.temperatureHumidifier, 10).c_str());
}

/**
 * \brief Read which buttons are pressed.
 *
 * \return the pressed button in a bitmask
 */
uint8_t HID::readButtons()
{
    ConfigurationIO *io = configuration.getIO();
    uint8_t buttons = 0;
    buttons |= digitalRead(io->buttonNext) ? NEXT : 0;
    buttons |= digitalRead(io->buttonSelect) ? SELECT : 0;
    return buttons;
}

/**
 * Calculate the time the pre-heat or running phase is running (in seconds)
 */
uint32_t HID::calculateTimeRunning()
{
    if (startTime != 0) {
        return (millis() - startTime) / 1000;
    }
    return 0;
}

/**
 * Calculate the time the current program stage is running (in seconds).
 */
uint32_t HID::calculateTimeRemaining()
{
    if (startTime != 0) {
        uint32_t timeRunning = calculateTimeRunning();
        uint32_t duration = (state == PREHEAT ? selectedProgram->durationPreHeat : selectedProgram->duration) * 60;
        if (timeRunning < duration) { // prevent under-flow
            return duration - timeRunning;
        }
    }
    return 0;
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
    sprintf(buffer, (char *)F("%d:%02d:%02d"), hours, minutes, (int) (seconds % 60));
    return String(buffer);
}

/**
 * Convert integer to decimal
 */
String HID::toDecimal(int16_t number, uint8_t divisor)
{
    char buffer[10];
    snprintf(buffer, 9, (char *)F("%d.%d"), number / divisor, abs(number % divisor));
    return String(buffer);
}

void HID::softReset()
{
    asm volatile ("  jmp 0");
}

void HID::addTime(uint16_t seconds)
{
}

void HID::switchToRunning()
{
}

/*
 * Convert the state into a string.
 */
String HID::stateToStr(State state)
{
    switch (state) {
    case UNKNOWN:
        return F("unknown");
    case READY:
        return F("ready");
    case PREHEAT:
        return F("pre-heating");
    case RUNNING:
        return F("running");
    case OVERTEMP:
        return F("over-temperature");
    case FINISHED:
        return F("shut-down");
    case FAULT:
        return F("error");
    }
    return F("invalid");
}


