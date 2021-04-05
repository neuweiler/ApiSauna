/*
 * Logger.cpp
 *
 * Logger to output formatted messages to the serial bus (monitoring / debugging)
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

#include "Logger.h"

Logger::LogLevel Logger::logLevel = CFG_DEFAULT_LOGLEVEL;
uint32_t Logger::lastLogTime = 0;
bool Logger::debugging = (Logger::logLevel == Debug);
char *Logger::msgBuffer = new char[CFG_LOG_BUFFER_SIZE];

/*
 * Output a debug message with a variable amount of parameters.
 * printf() style, see Logger::log()
 *
 */
void Logger::debug(String message, ...) {
	if (logLevel > Debug) {
		return;
	}

	va_list args;
	va_start(args, message);
	Logger::log(Debug, message, args);
	va_end(args);
}

/*
 * Output a info message with a variable amount of parameters
 * printf() style, see Logger::log()
 */
void Logger::info(String message, ...) {
	if (logLevel > Info) {
		return;
	}

	va_list args;
	va_start(args, message);
	Logger::log(Info, message, args);
	va_end(args);
}

/*
 * Output a warning message with a variable amount of parameters
 * printf() style, see Logger::log()
 */
void Logger::warn(String message, ...) {
	if (logLevel > Warn) {
		return;
	}

	va_list args;
	va_start(args, message);
	Logger::log(Warn, message, args);
	va_end(args);
}

/*
 * Output a error message with a variable amount of parameters
 * printf() style, see Logger::log()
 */
void Logger::error(String message, ...) {
	if (logLevel > Error) {
		return;
	}

	va_list args;
	va_start(args, message);
	Logger::log(Error, message, args);
	va_end(args);
}

/*
 * Output a comnsole message with a variable amount of parameters
 * printf() style, see Logger::logMessage()
 */
void Logger::console(String message, ...) {
	va_list args;
	va_start(args, message);
	vsnprintf(msgBuffer, CFG_LOG_BUFFER_SIZE, message.c_str(), args);
	Serial.println(msgBuffer);
	va_end(args);
}

/*
 * Set the log level. Any output below the specified log level will be omitted.
 * Also set the debugging flag for faster evaluation in isDebug().
 */
void Logger::setLoglevel(LogLevel level) {
	logLevel = level;
	debugging = (level == Debug);
}

/*
 * Retrieve the current log level.
 */
Logger::LogLevel Logger::getLogLevel() {
	return logLevel;
}

/*
 * Return a timestamp when the last log entry was made.
 */
uint32_t Logger::getLastLogTime() {
	return lastLogTime;
}

/*
 * Returns if debug log level is enabled. This can be used in time critical
 * situations to prevent unnecessary string concatenation (if the message won't
 * be logged in the end).
 *
 * Example:
 * if (Logger::isDebug()) {
 *    Logger::debug("current time: %d", millis());
 * }
 */
boolean Logger::isDebug() {
	return debugging;
}

/*
 * Output a log message (called by debug(), info(), warn(), error(), console())
 *
 * Supports printf() syntax
 */
void Logger::log(LogLevel level, String format, va_list args) {
	String logLevel = F("DEBUG");
	lastLogTime = millis();

	switch (level) {
	case Info:
		logLevel = F("INFO");
		break;
	case Warn:
		logLevel = F("WARNING");
		break;
	case Error:
		logLevel = F("ERROR");
		break;
	}
	vsnprintf(msgBuffer, CFG_LOG_BUFFER_SIZE, format.c_str(), args);

	// print to serial USB
	Serial.print(lastLogTime);
	Serial.print(F(" - "));
	Serial.print(logLevel);
	Serial.print(F(": "));
	Serial.println(msgBuffer);
}
