/*
 * Crc.cpp
 */

#include "Crc.h"

/**
 * Calculate the CRC of a memory area defined by a location and its size.
 */
uint32_t Crc::calculate(uint8_t *location, uint32_t size) {
	const unsigned long crcTable[16] = {
			0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
			0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
			0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
			0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };

	unsigned long crc = ~0L;

	for (uint8_t *pos = location; pos < location + size; ++pos) {
		crc = crcTable[(crc ^ *pos) & 0x0f] ^ (crc >> 4);
		crc = crcTable[(crc ^ (*pos >> 4)) & 0x0f] ^ (crc >> 4);
		crc = ~crc;
	}
	return crc;
}
