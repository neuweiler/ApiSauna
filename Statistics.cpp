/*
 * Statistics.cpp
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

#include "Statistics.h"


Statistics::Statistics()
{
    reset();
}

Statistics::~Statistics()
{
}

/**
 * Return the instance of the singleton
 */
Statistics *Statistics::getInstance()
{
    static Statistics instance;
    return &instance;
}

void Statistics::load()
{
    Logger::info(F("loading statistics"));
    EEPROM.get(CONFIG_ADDRESS_STATISTICS, *getStatistics());
    //TODO verify crc
}

void Statistics::save()
{
//TODO calc/update the crc's of all config
//    configUnused.crc;
    Logger::info(F("saving statistics"));
    EEPROM.put(CONFIG_ADDRESS_STATISTICS, *getStatistics());
}

void Statistics::reset()
{
    Logger::info(F("resetting stats"));

    StatisticValues *stats = getStatistics();

    stats->unused = 0;
}

StatisticValues *Statistics::getStatistics()
{
    static StatisticValues stats;
    return &stats;
}
