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
#include "SimpleList.h"
#include "Controller.h"
#include <LiquidCrystal.h>

enum Action {
    START_PROGRAM,
    MONITOR_HIVE,
    CONFIG_TEST1,
    CONFIG_TEST2,
    CONFIG_TEST3
};

class SubMenuEntry
{
public:
    String name;
    Action action;
};

class MenuEntry
{
public:
    String name;
    SimpleList<SubMenuEntry> subMenuEntries;
};

class HID
{
public:
    HID();
    virtual ~HID();
    void init();
    void loop();

private:
    enum Button
    {
        RIGHT,
        UP,
        DOWN,
        LEFT,
        SELECT,
        NONE
    };
    void displayProgramInfo();
    void logData();
    String convertTime(uint32_t millis);
    String toDecimal(int16_t number, uint8_t divisor);
    Button buttonPressed();
    void createMenu();
    void handleMenu(Button button);

    LiquidCrystal lcd = LiquidCrystal(0,0,0,0,0,0); // will be properly initialized later
    uint8_t controlPin;
    uint16_t baseValue;
    Button lastSelectedButton;
    Status::SystemState lastSystemState;
    SimpleList<MenuEntry> menuEntries;
    SimpleList<MenuEntry>::iterator itrMenu;
    SimpleList<SubMenuEntry>::iterator itrSubMenu;
    uint8_t tickCounter;
    char lcdBuffer[17];
};

extern HID hid;

#endif /* HID_H_ */
