#include "segment.h"
#include <Arduino.h>

BigDigits::BigDigits()
{
	bitNumber = 0;
	clkState = 1;
	dataState = 0;
	latchState = 1;
	byteNumber = 0;
	
}
void BigDigits::tick(void)
{
	if(byteNumber < 8)
	{
		if(bitNumber < 8)
		{
			//Work on current byte
			if(clkState == 0)
			{
				//rising edge
				clkState = 1;
				bitNumber++;
			}
			else
			{
				//Drop clock, change data
				clkState = 0;
				uint8_t tempBitNumber = 7 - bitNumber;
				dataState = ((byteArray[byteNumber]) >> tempBitNumber) & 0x01;
			}


		}
		else //bitnumber == 8
		{
			bitNumber = 0;
			byteNumber++;
		}
	}
	else
	{
		//Latch it
		latchState = 1;
	}
}

void BigDigits::reset(void)
{
	latchState = 0;
	bitNumber = 0;
	byteNumber = 0;
}

void BigDigits::setDigit( char input, uint8_t pos )
{
	if((input >= 0x30)&&(input <= 0x39))//ascii char
	{
		byteArray[pos] = bigDigitsLUT[input-0x30];
	}
	else
	{
		byteArray[pos] = bigDigitsLUT[10];
	}
}

