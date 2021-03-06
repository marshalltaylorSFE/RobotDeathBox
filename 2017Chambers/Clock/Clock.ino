#include <Arduino.h>
#include <stdint.h>
#include "Wire.h"

//**Big segment stuff*************************//
#include "segment.h"
#define segClockPin 9
#define segDataPin 8
#define segLatchPin 10
BigDigits myDigits;

#include "timerModule.h"
uint16_t maxTimer = 60000;
uint16_t maxInterval = 2000;

//**UART #defines*****************************//
#define REMOTELINKPORT Serial

#include <math.h>
#include "uCPacketClass.h"
#include "userPacketDefs.h"

//**Packets***********************************//
arenaPacket packet;

//**Serial Machine****************************//
uCPacketUART dataLinkHandler((HardwareSerial*)&REMOTELINKPORT, 64); //64 byte buffers

//Not used by this sketch but dependant on one 
#define LEDPIN 13

//If 328p based, do this
#ifdef __AVR__
#include <Arduino.h>
#endif

//If Teensy 3.1 based, do this
#ifdef __MK20DX256__
IntervalTimer myTimer;
#endif

//Globals
//**Copy to make a new timer******************//  
//TimerClass32 msTimerA( 200 ); //200 ms
TimerClass debugTimer( 1000 ); //milliseconds between calls
TimerClass segmentTimer( 1 );
TimerClass remoteInputTimer( 1 );

uint16_t msTicks = 0;
uint8_t msTicksLocked = 1; //start locked out

void setup()
{
	//Serial.begin(9600);
	pinMode(LEDPIN, OUTPUT);
	REMOTELINKPORT.begin(9600);
	REMOTELINKPORT.println("Program Started");

	myDigits.reset();
	myDigits.byteArray[0] = bigDigitsLUT[0];
	myDigits.byteArray[1] = bigDigitsLUT[1];
	myDigits.byteArray[2] = bigDigitsLUT[2];
	myDigits.byteArray[3] = bigDigitsLUT[3];
	myDigits.byteArray[4] = bigDigitsLUT[4];
	myDigits.byteArray[5] = bigDigitsLUT[5];
	myDigits.byteArray[6] = bigDigitsLUT[6];
	myDigits.byteArray[7] = bigDigitsLUT[7];
	pinMode(segClockPin, OUTPUT);
	pinMode(segDataPin, OUTPUT);
	pinMode(segLatchPin, OUTPUT);

	//If 328p based, do this
#ifdef __AVR__
	// initialize Timer1
	cli();          // disable global interrupts
	TCCR1A = 0;     // set entire TCCR1A register to 0
	TCCR1B = 0;     // same for TCCR1B

	// set compare match register to desired timer count:
	OCR1A = 16000;

	// turn on CTC mode:
	TCCR1B |= (1 << WGM12);

	// Set CS10 and CS12 bits for 1 prescaler:
	TCCR1B |= (1 << CS10);


	// enable timer compare interrupt:
	TIMSK1 |= (1 << OCIE1A);

	// enable global interrupts:
	sei();
#endif

	//If Teensy 3.1 based, do this
#ifdef __MK20DX256__
	// initialize IntervalTimer
	myTimer.begin(serviceMS, 1000);  // serviceMS to run every 0.001 seconds
#endif


}

void loop()
{
	//Update the timers, but only once per interrupt
	if( msTicksLocked == 0 )
	{
		//**Copy to make a new timer******************//  
		//msTimerA.update(msTicks);
		debugTimer.update(msTicks);
		segmentTimer.update(msTicks);
		remoteInputTimer.update(msTicks);
		
		//Done?  Lock it back up
		msTicksLocked = 1;
	}  //The ISR will unlock.

	//**Copy to make a new timer******************//  
	//if(msTimerA.flagStatus() == PENDING)
	//{
	//	//User code
	//}
	//**Read the input packet*********************//  
	if(remoteInputTimer.flagStatus() == PENDING)
	{
		dataLinkHandler.burstReadInputBuffer();
		if( dataLinkHandler.available() == sizeof packet )
		{
			dataLinkHandler.getPacket( (uint8_t *)&packet, sizeof packet );
			//Now do operations on returned packet
			//if( packet.someVar == blargle ) ...
			myDigits.setDigit(packet.byteArray[0], 0);
			myDigits.setDigit(packet.byteArray[1], 1);
			myDigits.setDigit(packet.byteArray[2], 2);
			myDigits.setDigit(packet.byteArray[3], 3);
			myDigits.setDigit(packet.byteArray[0], 4);
			myDigits.setDigit(packet.byteArray[1], 5);
			myDigits.setDigit(packet.byteArray[2], 6);
			myDigits.setDigit(packet.byteArray[3], 7);
			myDigits.reset();
		}
		else
		{
			dataLinkHandler.abandonRxPacket();
		}
	}	
	if(debugTimer.flagStatus() == PENDING)
	{
		//User code
		digitalWrite( LEDPIN, digitalRead( LEDPIN ) ^ 0x01 );
	}
//	if(segmentTimer.flagStatus() == PENDING)
	{
		//Sneak in the digit stuff here
		myDigits.tick();
		digitalWrite(segClockPin, myDigits.clkState);
		digitalWrite(segDataPin, myDigits.dataState);
		digitalWrite(segLatchPin, myDigits.latchState);
	}
}

//If 328p based, do this
#ifdef __AVR__
ISR(TIMER1_COMPA_vect)
#else
#endif
//If Teensy 3.1 based, do this
#ifdef __MK20DX256__
void serviceMS(void)
#else
#endif
{
	uint32_t returnVar = 0;
	if( msTicks >= ( maxTimer + maxInterval ) )
	{
		returnVar = msTicks - maxTimer;

	}
	else
	{
		returnVar = msTicks + 1;
	}
	msTicks = returnVar;
	msTicksLocked = 0;  //unlock
}

