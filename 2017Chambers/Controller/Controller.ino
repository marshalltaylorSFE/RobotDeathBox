#include <Arduino.h>
#include <stdint.h>
#include "Wire.h"

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

#include "CPanel.h" //Edit and rename this

CPanel myCustomPanel;

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
TimerClass halfSecondTimer( 500 ); //milliseconds between calls
uint8_t secondToggler = 0;
TimerClass segmentTimer( 3 );
TimerClass remoteInputTimer( 3 );
TimerClass serialSendTimer( 100 );

uint16_t msTicks = 0;
uint8_t msTicksLocked = 1; //start locked out
uint8_t pauseFlag = 0;

void setup()
{
	//Serial.begin(9600);
	pinMode(LEDPIN, OUTPUT);
	REMOTELINKPORT.begin(9600);
	//REMOTELINKPORT.println("Program Started");

	myCustomPanel.reset();
	
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
		halfSecondTimer.update(msTicks);
		debugTimer.update(msTicks);
		segmentTimer.update(msTicks);
		remoteInputTimer.update(msTicks);
		serialSendTimer.update(msTicks);
		
		//Done?  Lock it back up
		msTicksLocked = 1;
	}  //The ISR will unlock.
	if(halfSecondTimer.flagStatus() == PENDING)
	{ 
		secondToggler ^= 0x01;
		if(myCustomPanel.timerRunning)
		{
			if((myCustomPanel.timeRemaining > 0)&&(secondToggler == 1))
			{
				myCustomPanel.timeRemaining--;
				if(myCustomPanel.timeRemaining == 0)
				{
					myCustomPanel.timerRunning = 0;
					myCustomPanel.timeExpired = 1;
				}
				//Serial.println(myCustomPanel.timeRemaining);
			}
		}
		if((myCustomPanel.displayOn == 0)||((secondToggler == 0) && myCustomPanel.displayFlashing == 1))
		{
			packet.byteArray[0] = ' ';
			packet.byteArray[1] = ' ';
			packet.byteArray[2] = ' ';
			packet.byteArray[3] = ' ';
		}
		else
		{
			char buffer[5];
			sprintf(buffer, "%2d", myCustomPanel.timeRemaining / 60);
			packet.byteArray[2] = buffer[1];
			packet.byteArray[3] = buffer[0];
			if(myCustomPanel.timeRemaining < 60)
			{
				packet.byteArray[2] = ' ';
				packet.byteArray[3] = ' ';
			}
			if(myCustomPanel.timeRemaining > 9)
			{
				sprintf(buffer, "%02d", myCustomPanel.timeRemaining % 60);
			}
			else
			{
				sprintf(buffer, "%2d", myCustomPanel.timeRemaining % 60);
			}
			packet.byteArray[0] = buffer[1];
			packet.byteArray[1] = buffer[0];
		}
	}
	if(serialSendTimer.flagStatus() == PENDING)
	{
		dataLinkHandler.sendPacket( (uint8_t *)&packet, sizeof packet );
	}
	//**Read the input packet*********************//  
	if(remoteInputTimer.flagStatus() == PENDING)
	{
		dataLinkHandler.burstReadInputBuffer();
		if( dataLinkHandler.available() == sizeof packet )
		{
			dataLinkHandler.getPacket( (uint8_t *)&packet, sizeof packet );
			//Now do operations on returned packet
			//if( packet.someVar == blargle ) ...
//			myDigits.byteArray[0] = bigDigitsLUT[packet.byteArray[0]];
//			myDigits.byteArray[1] = bigDigitsLUT[packet.byteArray[1]];
//			myDigits.byteArray[2] = bigDigitsLUT[packet.byteArray[2]];
//			myDigits.byteArray[3] = bigDigitsLUT[packet.byteArray[3]];
//			myDigits.byteArray[4] = bigDigitsLUT[packet.byteArray[4]];
//			myDigits.byteArray[5] = bigDigitsLUT[packet.byteArray[5]];
//			myDigits.byteArray[6] = bigDigitsLUT[packet.byteArray[6]];
//			myDigits.byteArray[7] = bigDigitsLUT[packet.byteArray[7]];
//			myDigits.reset();
		}
	}	
	if(debugTimer.flagStatus() == PENDING)
	{
		//User code
		digitalWrite( LEDPIN, digitalRead( LEDPIN ) ^ 0x01 );
	}
	if(segmentTimer.flagStatus() == PENDING)
	{
		myCustomPanel.tickStateMachine(3);

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
