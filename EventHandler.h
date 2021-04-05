/*
 * EventHandler.h
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

#ifndef EVENTHANDLER_H_
#define EVENTHANDLER_H_

#include <Arduino.h>
#include "SimpleList.h"
#include "EventListener.h"
#include "Logger.h"

class EventHandler
{
public:
    EventHandler();
    virtual ~EventHandler();
    void subscribe(EventListener *listener);
    void publish(EventListener::Event event, ...);
private:
    EventHandler(EventHandler const&); // copy disabled
    void operator=(EventHandler const&); // assignment disabled

    SimpleList<EventListener *> listeners;
};

extern EventHandler eventHandler;

#endif /* EVENTHANDLER_H_ */
