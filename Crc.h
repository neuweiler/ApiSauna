/*
 * Crc.h
 *
 *  Created on: 25 May 2018
 *      Author: michaeln
 */

#ifndef CRC_H_
#define CRC_H_

#include <Arduino.h>

class Crc
{
public:
    static uint32_t calculate(uint8_t *location, uint32_t size);
};

#endif /* CRC_H_ */
