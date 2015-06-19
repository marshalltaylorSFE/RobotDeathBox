//**********************************************************************//
//  BEERWARE LICENSE
//
//  This code is free for any use provided that if you meet the author
//  in person, you buy them a beer.
//
//  This license block is BeerWare itself.
//
//  Written by:  Marshall Taylor
//  Created:  June 17, 2015
//
//**********************************************************************//
#include "Arduino.h"
#include "SoftwareSerial.h"

//*****How to operate TimerClass**************//
//  Make TimerClass objects for each thing that needs periodic service
//  pass the interval of the period in ticks
//  Set MAXINTERVAL to the max foreseen interval of any TimerClass
//  Set MAXTIMER to overflow number in the header.  MAXTIMER + MAXINTERVAL
//    cannot exceed variable size.

//**General***********************************//
#include "stdint.h"
SoftwareSerial mySerial(8,9);

//**Timers and stuff**************************//
#include "timerModule.h"

//Use IntervalTimer for Teensy
//IntervalTimer myTimer;

TimerClass debugTimer(1000);

uint16_t msTicks = 0;
uint8_t msTicksMutex = 1; //start locked out

#define MAXINTERVAL 2000 //Max TimerClass interval

void setup()
{
  delay(500);

  Serial.begin(9600);
  //HW port is for winners.
  //Serial.println("Program started\n");
  
  mySerial.begin(9600);
  mySerial.print("Program started\n");

  // initialize IntervalTimer interrupt stuff
  
  // serviceMS to run every 0.001 seconds -- Teensy
  //myTimer.begin(serviceMS, 1000);
  
  // initialize Timer1 -- Arduino
  cli();          // disable global interrupts
  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B
  OCR1A = 16000; // set compare match register to desired timer count
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1 << CS10); // Set CS10 and CS12 bits for 1 prescaler
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei(); // enable global interrupts
  
  
}

void loop()
{
  // main program
  
  if( msTicksMutex == 0 )  //Only touch the timers if clear to do so.
  {
//**Copy to make a new timer******************//  
//    msTimerA.update(msTicks);
	debugTimer.update(msTicks);

 //Done?  Lock it back up
    msTicksMutex = 1;
  }  //The ISR should clear the mutex.
  
//**Copy to make a new timer******************//  
//  if(msTimerA.flagStatus() == PENDING)
//  {
//    digitalWrite( LEDPIN, digitalRead(LEDPIN) ^ 1 );
//  }
  if(debugTimer.flagStatus() == PENDING)
  {
	//Build and send a fake-o packet
	Serial.print("~");
	Serial.print("0");
	Serial.print("0");
	Serial.print("0");
	Serial.print("1");
	Serial.print(" ");
	Serial.print(" ");
	Serial.print(" ");
	Serial.print(" ");
	Serial.print(" ");
	Serial.print(" ");
	Serial.print("A");
	Serial.print("4");
	Serial.print("3");
	Serial.print("2");
	Serial.print("1");
	Serial.print("0");
	Serial.print("0");
	Serial.print("B");
	Serial.print("E");
	Serial.print("E");
	Serial.print("F");
	Serial.print(".");//Dummy byte bugfix-- shouldn't be needed
	Serial.write(0x0D);
	Serial.write(0x0A);

  }  
  
}

//void serviceMS(void) // Teensy timer
ISR(TIMER1_COMPA_vect) // 328p timer
{
  uint32_t returnVar = 0;
  if(msTicks >= ( MAXTIMER + MAXINTERVAL ))
  {
    returnVar = msTicks - MAXTIMER;

  }
  else
  {
    returnVar = msTicks + 1;
  }
  msTicks = returnVar;
  msTicksMutex = 0;  //unlock
  
}
