/*
 * Controller.cpp
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

#include "EventHandler.h"

EventHandler eventHandler;

EventHandler::EventHandler() {
}

EventHandler::~EventHandler() {
}

/**
 * Attach an listener
 */
void EventHandler::subscribe(EventListener *listener) {
	listeners.push_back(listener);
}

/**
 * Send an event to all attached/subscribed listeners
 */
void EventHandler::publish(EventListener::Event event, ...) {
	va_list args;
	va_start(args, event);
	for (SimpleList<EventListener*>::iterator entry = listeners.begin(); entry != listeners.end(); ++entry) {
		if (logger.isDebug()) {
			logger.debug(F("publishing event %x to %x"), event, *entry);
		}
		((EventListener*) *entry)->handleEvent(event, args);
	}
	va_end(args);
}
