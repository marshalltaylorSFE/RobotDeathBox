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
#include "HOS_char.h"

//**Arena pin #defines************************//
//DOORS

#define DOORSENSOR_MAIN  25
#define DOORSENSOR_REAR  24
#define DOORLOCK_MAIN    26
#define DOORLOCK_REAR    27

//ARENA LEDS

//In PANEL

//USERBUTTONS

//DEFINED IN PANEL
// #define  BLUESQUARE_TAPOUT_BUTTON_INPUT  35
// #define  BLUESQUARE_READY_BUTTON_INPUT   34

// #define  REDSQUARE_TAPOUT_BUTTON_INPUT  23
// #define  REDSQUARE_READY_BUTTON_INPUT   22

#define  BLUESQUARE_TAPOUT_BUTTON_LED    30
#define  BLUESQUARE_READY_BUTTON_LED     31

#define  REDSQUARE_TAPOUT_BUTTON_LED    32
#define  REDSQUARE_READY_BUTTON_LED     33
#define hazardPin 36

//*****How to operate TimerClass**************//
//  Make TimerClass objects for each thing that needs periodic service
//  pass the interval of the period in ticks
//  Set MAXINTERVAL to the max foreseen interval of any TimerClass
//  Set MAXTIMER to overflow number in the header.  MAXTIMER + MAXINTERVAL
//    cannot exceed variable size.

//**General***********************************//
#include "stdint.h"
//SoftwareSerial mySerial(8,9);

//**Timers and stuff**************************//
#include "timerModule.h"

//Use IntervalTimer for Teensy
//IntervalTimer myTimer;

TimerClass sendPacketTimer(100);
TimerClass serialReadCheckTimer( 1 );
TimerClass processSMTimer( 50 );
TimerClass segmentTimer( 1 );

uint16_t msTicks = 0;
uint8_t msTicksMutex = 1; //start locked out

//Timer for the match counter (accurate)
TimerClass matchTimer(1000);
//TimeKeeper matchCounter(); in state machine

#define MAXINTERVAL 2000 //Max TimerClass interval

//Panel related variables
#include "Panel.h"
Panel myArenaPanel;
TimerClass debounceTimer(5);
uint8_t ledToggleState = 1;
TimerClass panelUpdateTimer(10);

//**Packet handling stuff*********************//
#define PACKET_LENGTH 24
#define START_SYMBOL '~'

char lastchar;
char rxPacket[PACKET_LENGTH];
char txPacket[PACKET_LENGTH];
char lastPacket[PACKET_LENGTH];
char packetPending = 0;

char packet_ptr;
//**Big segment stuff*************************//
#include "segment.h"
#define segClockPin 9
#define segDataPin 8
#define segLatchPin 10
BigDigits myDigits;


//**Arena State Machine***********************//
#include "arenaStateMachine.h"
ArenaStateMachine myArena;

void setup()
{
  delay(500);

  Serial1.begin(9600);
  
  Serial.begin(9600);
  Serial.print("Program started\n");

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
  
  //Build the empty packet
  txPacket[0] = '~';
  txPacket[1] = '0';
  txPacket[2] = '0';
  txPacket[3] = '0';
  txPacket[4] = '0';
  txPacket[5] = ' ';
  txPacket[6] = ' ';
  txPacket[7] = ' ';
  txPacket[8] = ' ';
  txPacket[9] = ' ';
  txPacket[10] = ' ';
  txPacket[11] = ' ';
  txPacket[12] = 'F';
  txPacket[13] = '0';
  txPacket[14] = '0';
  txPacket[15] = '0';
  txPacket[16] = ' ';
  txPacket[17] = ' ';
  txPacket[18] = ' ';
  txPacket[19] = ' ';
  txPacket[20] = ' ';
  txPacket[21] = ' ';
  txPacket[22] = 0x0D;
  txPacket[23] = 0x0A;
  
  //Configure the arena facing HW
  //DOORS

  pinMode( DOORSENSOR_MAIN, INPUT_PULLUP );
  pinMode( DOORSENSOR_REAR, INPUT_PULLUP );
  pinMode( DOORLOCK_MAIN, OUTPUT);
  pinMode( DOORLOCK_REAR, OUTPUT);
  
  //USERBUTTONS

  //DEFINED IN PANEL  
  //pinMode( BLUESQUARE_TAPOUT_BUTTON_INPUT, INPUT );
  pinMode( BLUESQUARE_TAPOUT_BUTTON_LED, OUTPUT);
  //pinMode( BLUESQUARE_READY_BUTTON_INPUT, INPUT );
  pinMode( BLUESQUARE_READY_BUTTON_LED, OUTPUT);

  //pinMode( REDSQUARE_TAPOUT_BUTTON_INPUT, INPUT );
  pinMode( REDSQUARE_TAPOUT_BUTTON_LED, OUTPUT);
  //pinMode( REDSQUARE_READY_BUTTON_INPUT, INPUT );
  pinMode( REDSQUARE_READY_BUTTON_LED, OUTPUT);

  pinMode( hazardPin, OUTPUT );
  //ARENA LEDS

  pinMode(BLUESQUARE_RED, OUTPUT);
  pinMode(BLUESQUARE_GREEN, OUTPUT);
  pinMode(BLUESQUARE_BLUE, OUTPUT);

  pinMode(REDSQUARE_RED, OUTPUT);
  pinMode(REDSQUARE_GREEN, OUTPUT);
  pinMode(REDSQUARE_BLUE, OUTPUT);

  pinMode(MAINDOOR_RED, OUTPUT);
  pinMode(MAINDOOR_GREEN, OUTPUT);
  pinMode(MAINDOOR_BLUE, OUTPUT);

  pinMode(REARDOOR_RED, OUTPUT);
  pinMode(REARDOOR_GREEN, OUTPUT);
  pinMode(REARDOOR_BLUE, OUTPUT);

  myArenaPanel.init();
  
  myDigits.reset();
  myDigits.byteArray[0] = bigDigitsLUT[0];
  myDigits.byteArray[1] = bigDigitsLUT[1];
  myDigits.byteArray[2] = bigDigitsLUT[2];
  myDigits.byteArray[3] = bigDigitsLUT[3];
  myDigits.byteArray[4] = bigDigitsLUT[4];
  myDigits.byteArray[5] = bigDigitsLUT[5];
  pinMode(segClockPin, OUTPUT);
  pinMode(segDataPin, OUTPUT);
  pinMode(segLatchPin, OUTPUT);
  
}

void loop()
{
  // main program
  
  if( msTicksMutex == 0 )  //Only touch the timers if clear to do so.
  {
//**Copy to make a new timer******************//  
//    msTimerA.update(msTicks);
    processSMTimer.update(msTicks);
	sendPacketTimer.update(msTicks);
	serialReadCheckTimer.update(msTicks);
	debounceTimer.update(msTicks);
	 panelUpdateTimer.update(msTicks);
	 segmentTimer.update(msTicks);
 //Done?  Lock it back up
    msTicksMutex = 1;
  }  //The ISR should clear the mutex.
  
//**Copy to make a new timer******************//  
//  if(msTimerA.flagStatus() == PENDING)
//  {
//    digitalWrite( LEDPIN, digitalRead(LEDPIN) ^ 1 );
//  }
//**Copy to make a new timer******************//  
 //Put this in the main loop      if(segmentTimer.flagStatus() == PENDING)
 {
	  //Sneak in the digit stuff here
	  myDigits.tick();
	  digitalWrite(segClockPin, myDigits.clkState);
	  digitalWrite(segDataPin, myDigits.dataState);
	  digitalWrite(segLatchPin, myDigits.latchState);
	  // Serial.print(myDigits.clkState, HEX);
	  // Serial.print(myDigits.dataState, HEX);
	  // Serial.print(myDigits.latchState, HEX);
	  // Serial.print("\n");
	  
 }
//**Debounce timer****************************//  
 if(debounceTimer.flagStatus() == PENDING)
 {
   myArenaPanel.timersMIncrement(5);

 }
//**State machine process timer***************//  
  if(processSMTimer.flagStatus() == PENDING)
  {
    myArena.tick();
	//Pure logic stuff
	myArena.doorsLeftAjar = 0x01 ^ digitalRead( DOORSENSOR_MAIN );
	myArena.doorsRightAjar = 0x01 ^ digitalRead( DOORSENSOR_REAR );
	myArena.doorsLeftAjarLed = myArena.doorsLeftAjar;
	myArena.doorsRightAjarLed = myArena.doorsRightAjar;
	digitalWrite( hazardPin, myArena.hazardsOn );
	//Locks
	digitalWrite( DOORLOCK_MAIN, (0x01 ^ myArena.doorsLocked));
	digitalWrite( DOORLOCK_REAR, (0x01 ^ myArena.doorsLocked));
	
	//Player buttons
	switch( myArena.redSquareLeds)
	{
		case 1:
			digitalWrite( REDSQUARE_TAPOUT_BUTTON_LED, 0);
			digitalWrite( REDSQUARE_READY_BUTTON_LED, 1);
		break;
		case 2:
			digitalWrite( REDSQUARE_TAPOUT_BUTTON_LED, 1);
			digitalWrite( REDSQUARE_READY_BUTTON_LED, 0);
		break;
		default:
		case 0:
			digitalWrite( REDSQUARE_TAPOUT_BUTTON_LED, 0);
			digitalWrite( REDSQUARE_READY_BUTTON_LED, 0);
		break;
	}
	switch( myArena.blueSquareLeds)
	{
		case 1:
			digitalWrite( BLUESQUARE_TAPOUT_BUTTON_LED, 0);
			digitalWrite( BLUESQUARE_READY_BUTTON_LED, 1);
		break;
		case 2:
			digitalWrite( BLUESQUARE_TAPOUT_BUTTON_LED, 1);
			digitalWrite( BLUESQUARE_READY_BUTTON_LED, 0);
		break;
		default:
		case 0:
			digitalWrite( BLUESQUARE_TAPOUT_BUTTON_LED, 0);
			digitalWrite( BLUESQUARE_READY_BUTTON_LED, 0);
		break;
	}	
	
  }

//**Update the panel LEDs and stuff***********//  
 if(panelUpdateTimer.flagStatus() == PENDING)
 {
   myArenaPanel.update();
	if( myArenaPanel.redSquareReady.newData )
	{
		//Serial.println("redSquareReady");
		if( myArenaPanel.redSquareReady.getState() == 1 )
		{
			Serial.print("setting bit");
			myArena.redSquareReady = 1;
		}
	}
	if( myArenaPanel.redSquareTapOut.newData )
	{
		//Serial.println("redSquareTapOut");
		if( myArenaPanel.redSquareTapOut.getState() == 1 )
		{
			myArena.redSquareTapOut = 1;
		}
	}
	if( myArenaPanel.blueSquareReady.newData )
	{
		//Serial.println("blueSquareReady");
		if( myArenaPanel.blueSquareReady.getState() == 1 )
		{
			myArena.blueSquareReady = 1;
		}
	}
	if( myArenaPanel.blueSquareTapOut.newData )
	{
		//Serial.println("blueSquareTapOut");
		if( myArenaPanel.blueSquareTapOut.getState() == 1 )
		{
			myArena.blueSquareTapOut = 1;
		}
	}
 }
  if(matchTimer.flagStatus() == PENDING)
  {
	  if( myArena.matchRunning == 1 )
	  {
		  myArena.matchCounter.mDecrement(1);
		  myArena.countoutCounter.mDecrement(1);
	  }
  }
  if(sendPacketTimer.flagStatus() == PENDING)
  {
	//Build and send a fake-o packet
	// Serial1.print("~");
	// Serial1.print("0");
	// Serial1.print("0");
	// Serial1.print("0");
	// Serial1.print("1");
	// Serial1.print(" ");
	// Serial1.print(" ");
	// Serial1.print(" ");
	// Serial1.print(" ");
	// Serial1.print(" ");
	// Serial1.print(" ");
	// Serial1.print("A");
	// Serial1.print("4");
	// Serial1.print("9");
	// Serial1.print("2");
	// Serial1.print("1");
	// Serial1.print("0");
	// Serial1.print("0");
	// Serial1.print("B");
	// Serial1.print("E");
	// Serial1.print("E");
	// Serial1.print("F");
	// Serial1.print(".");//Dummy byte bugfix-- shouldn't be needed
	// Serial1.write(0x0D);
	// Serial1.write(0x0A);
	//Serial1.write(0x0D);
	//Serial1.write(0x0A);
	char tempString[10]; // for the time decimal conversion
    // Magical sprintf creates a string for us to send to the s7s.
    //  The %4d option creates a 4-digit integer.
	tempString[0] = ' ';
	tempString[1] = ' ';
	tempString[2] = ' ';
	tempString[3] = ' ';
	
	//copy the time stuff
	uint16_t tempCounter = myArena.matchCounter.mGet();
	uint16_t tempTime = 0;
	tempTime = (tempCounter / 60 );
	tempTime *= 100;
	tempTime += (tempCounter % 60);
	switch( myArena.displayMode )
	{
		case 1:
			sprintf(tempString, "%4d", (int)tempTime);
			txPacket[10] = '1';
			txPacket[11] = '0';
			txPacket[12] = tempString[0];
			txPacket[13] = tempString[1];
			txPacket[14] = tempString[2];
			txPacket[15] = tempString[3];
			myDigits.byteArray[0] = bigDigitsLUT[char2hex(tempString[3])];
			myDigits.byteArray[1] = bigDigitsLUT[char2hex(tempString[2])];
			myDigits.byteArray[2] = bigDigitsLUT[char2hex(tempString[1])];
			myDigits.byteArray[3] = bigDigitsLUT[char2hex(tempString[3])];
			myDigits.byteArray[4] = bigDigitsLUT[char2hex(tempString[2])];
			myDigits.byteArray[5] = bigDigitsLUT[char2hex(tempString[1])];
			break;
		case 2:
		    sprintf(tempString, "%4d", (int)myArena.countoutCounter.mGet());
			txPacket[10] = '0';
			txPacket[11] = '0';
			txPacket[12] = '-';
			txPacket[13] = tempString[2];
			txPacket[14] = tempString[3];
			txPacket[15] = '-';
			myDigits.byteArray[0] = bigDigitsLUT[char2hex(' ')];
			myDigits.byteArray[1] = bigDigitsLUT[char2hex(tempString[3])];
			myDigits.byteArray[2] = bigDigitsLUT[char2hex(tempString[2])];
			myDigits.byteArray[3] = bigDigitsLUT[char2hex(' ')];
			myDigits.byteArray[4] = bigDigitsLUT[char2hex(tempString[3])];
			myDigits.byteArray[5] = bigDigitsLUT[char2hex(tempString[2])];
			break;
		default:
			txPacket[10] = '0';
			txPacket[11] = '0';
			txPacket[12] = '-';
			txPacket[13] = '-';
			txPacket[14] = '-';
			txPacket[15] = '-';			
		break;
	}
	myDigits.reset();
	
	//Packetize the LED data
	uint8_t tempData = 0;
	tempData |= (myArena.hazardsOffLed << 2);
	tempData |= (myArena.hazardsActiveLed);
	txPacket[1] = hex2char(tempData);
	tempData = 0;
	tempData |= (myArena.matchReadyLed << 2);
	tempData |= (myArena.matchPauseLed);
	txPacket[2] = hex2char(tempData);
	tempData = 0;
	tempData |= (myArena.doorsLeftAjarLed << 2);
	tempData |= (myArena.doorsRightAjarLed);
	txPacket[3] = hex2char(tempData);	
	tempData = 0;
	tempData |= (myArena.doorsUnlockLed << 2);
	tempData |= (0);
	txPacket[4] = hex2char(tempData);
	
	for(int i = 0; i < PACKET_LENGTH; i++)
	{
		Serial1.write(txPacket[i]);
	}
	Serial1.write(0x0D);
	Serial1.write(0x0A);
	
	
  } 
  //**Ckeck serial input buffer*****************//  
  if(serialReadCheckTimer.flagStatus() == PENDING)
  {
	  //This is the actual serial stuff
    if (Serial1.available())
    {
      lastchar = Serial1.read();
      //look for packet start (START_SYMBOL)
      if( lastchar == START_SYMBOL )
      {
		  //Serial1.println("START SYMBOL");
        //Flag that the packet needs to be serviced
        packetPending = 1;
        
        //Fill packet with null, reset the pointer
        for( int i = 0; i < PACKET_LENGTH; i++ )
        {
          rxPacket[i] = 0;
        }
        //write the start char
        rxPacket[0] = START_SYMBOL;
        //reset the pointer
        packet_ptr = 1;
      }
      else
      if( ( packet_ptr < PACKET_LENGTH ) && (packet_ptr > 0) )//check for room in the packet, and that the start char has been seen
      {
		  //Serial1.print("DATA: ");
        //put the char in the packet
        rxPacket[packet_ptr] = lastchar;
		//Serial1.println(rxPacket[packet_ptr]);
		//rxPacket[packet_ptr] = char2hex(rxPacket[packet_ptr]);
        //advance the pointer
        packet_ptr++;
        //turn on LED
      }
      else
      {
		  //Serial1.println("END");
        //Just overwrite to the last position
        rxPacket[PACKET_LENGTH - 1] = lastchar;
      }
    }
    
    //if the packet is full and the last char is LF or CR, *do something here*
    if((packetPending == 1) && ((packet_ptr == (PACKET_LENGTH)) && ((rxPacket[PACKET_LENGTH - 1] == 0x0A) || (rxPacket[PACKET_LENGTH - 1] == 0x0D))) )
    {
		//Serial1.println("SERVICING PENDING");
		packetPending = 0;
		//Print the packet to software serial
		// for(int i = 0; i < PACKET_LENGTH; i++)
		// {
			// mySerial.write(rxPacket[i]);
		// }
		//Now update internal variables
		myArena.hazardsSwitch = ( char2hex(rxPacket[7]) >> 3 ) & 0x01;
		myArena.doorsSwitch = ( char2hex(rxPacket[7]) >> 2 ) & 0x01;
		if( (( char2hex(rxPacket[5]) >> 3 ) & 0x01) == 1 )//If the bit is set,
		{
			myArena.judgesReady = 1; //Can only set, not clear
		}
		if( (( char2hex(rxPacket[5]) >> 2 ) & 0x01) == 1 )//If the bit is set,
		{
			myArena.matchPause = 1; //Can only set, not clear
		}
		if( (( char2hex(rxPacket[5]) >> 1 ) & 0x01) == 1 )//If the bit is set,
		{
			myArena.countStart = 1; //Can only set, not clear
		}
		if( (( char2hex(rxPacket[5]) ) & 0x01) == 1 )//If the bit is set,
		{
			myArena.countReset = 1; //Can only set, not clear
		}
		if( (( char2hex(rxPacket[6]) >> 3 ) & 0x01) == 1 )//If the bit is set,
		{
			myArena.eStop = 1; //Can only set, not clear
		}

    }
  }//End checking-for-serial timer  
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
  
  //Match timer in the interrupt
  matchTimer.update(msTicks);
}
