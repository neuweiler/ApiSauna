/*
 * HID.h
 *
 * Main part of the bee sauna control software. It initializes all devices
 * and controls the main loop.
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

#ifndef HID_H_
#define HID_H_

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "SimpleList.h"
#include "Device.h"
#include "ProgramHandler.h"
#include "Beeper.h"

class HID: Device
{
public:
    HID();
    void initialize();
    void process();

private:
    enum Button
    {
        NEXT    = 1 << 0,
        SELECT  = 1 << 1
    };

    void displayProgramInfo();
    void logData();
    String convertTime(uint32_t millis);
    String toDecimal(int16_t number, uint8_t divisor);
    uint8_t readButtons();
    void handleProgramMenu();
    void printProgramMenu();
    void handleProgramInput();
    void handleFinishedInput();
    void checkReset();
    void displayHiveTemperatures(uint8_t row, bool displayAll);
    bool modal(String request, String negative, String positive, uint8_t timeout);
    void stateSwitch(Status::SystemState fromState, Status::SystemState toState);
    void softReset();
    void printFinishedMenu();

    LiquidCrystal lcd = LiquidCrystal(0, 0, 0, 0, 0, 0); // will be properly initialized later
    Status::SystemState lastSystemState;
    SimpleList<Program>::iterator selectedProgram;
    uint8_t tickCounter;
    uint8_t lastButtons;
    uint32_t resetStamp;
    char lcdBuffer[21];
    bool statusLed;
    Beeper beeper;
};

#endif /* HID_H_ */
