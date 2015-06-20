#ifndef SEGMENTS_H
#define SEGMENTS_H
#include "stdint.h"

const uint8_t bigDigitsLUT [11] = 
{
0x7b,
0x60,
0x5d,
0x75,
0x66,
0x37,
0x3f,
0x61,
0x7f,
0x67,
0x00
};

class BigDigits
{
public:
	uint8_t bitNumber;
	uint8_t byteNumber;
	uint8_t byteArray[6];
	uint8_t clkState;
	uint8_t dataState;
	uint8_t latchState;
	
	BigDigits();
	void tick(void);
	void reset(void);
	
};


#endif

