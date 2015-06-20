//**********************************************************************//
//  BEERWARE LICENSE
//
//  This code is free for any use provided that if you meet the author
//  in person, you buy them a beer.
//
//  This license block is BeerWare itself.
//
//  Written by:  Marshall Taylor
//  Created:  March 21, 2015
//
//**********************************************************************//
#include "HOS_char.h"

//**Panels and stuff**************************//

#include "Panel.h"
#include "PanelComponents.h"

//Panel related variables
Panel myPanel;

//**Timers and stuff**************************//
#include "timerModule.h"

#define MAXINTERVAL 2000 //Max TimerClass interval

//HOW TO OPERATE
//  Make TimerClass objects for each thing that needs periodic service
//  pass the interval of the period in ticks
//  Set MAXINTERVAL to the max foreseen interval of any TimerClass
//  Set MAXTIMER to overflow number in the header.  MAXTIMER + MAXINTERVAL
//    cannot exceed variable size.

TimerClass debugTimerClass( 333 );
TimerClass serialReadCheckTimer( 1 );
TimerClass panelUpdateTimer(10);
uint8_t debugLedStates = 1;

TimerClass ledToggleTimer( 333 );
uint8_t ledToggleState = 0;

TimerClass debounceTimer(5);
#include "timeKeeper.h"


//tick variable for interrupt driven timer1
uint16_t msTicks = 0;
uint8_t msTicksMutex = 1; //start locked out

//**Packet handling stuff*********************//
#define PACKET_LENGTH 24
#define START_SYMBOL '~'

char lastchar;
char rxPacket[PACKET_LENGTH];
char txPacket[PACKET_LENGTH];
char lastPacket[PACKET_LENGTH];
char packetPending = 0;

char packet_ptr;

//**Seven Segment Display*********************//
// Here we'll define the I2C address of our S7S. By default it
//  should be 0x71. This can be changed, though.
#include "Wire.h"
const byte s7sAddress = 0x71;
char tempString[10];  // Will be used with sprintf to create strings

// -----------------------------------------------------------------------------
void setup() 
{
  //Initialize serial:
  Serial.begin(9600);

  //Init panel.h stuff
  myPanel.init();
  
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
  
  //Debug setting of random states and stuff
  
  //Set all LED on
  myPanel.hazardsOffLed.setState(LEDON);
  myPanel.hazardsActiveLed.setState(LEDON);
  myPanel.matchReadyLed.setState(LEDON);
  myPanel.matchPauseLed.setState(LEDON);
  myPanel.doorsUnlockLed.setState(LEDON);
  myPanel.doorsLeftAjarLed.setState(LEDON);
  myPanel.doorsRightAjarLed.setState(LEDON);
  myPanel.update();
  
  //while(1);
  
  //Set all LED off
  // myPanel.hazardsOffLed.setState(LEDOFF);
  // myPanel.hazardsActiveLed.setState(LEDOFF);
  // myPanel.matchReadyLed.setState(LEDOFF);
  // myPanel.matchPauseLed.setState(LEDOFF);
  // myPanel.doorsUnlockLed.setState(LEDOFF);
  // myPanel.doorsLeftAjarLed.setState(LEDOFF);
  // myPanel.doorsRightAjarLed.setState(LEDOFF);
  // myPanel.update();
  
  //debugTimeKeeper.mClear();
  
  Wire.begin();  // Initialize hardware I2C pins
  // Clear the display, and then turn on all segments and decimals
  clearDisplayI2C();  // Clears display, resets cursor
  setBrightnessI2C(255);  // High brightness
    // Magical sprintf creates a string for us to send to the s7s.
  //  The %4d option creates a 4-digit integer.
  sprintf(tempString, "%4d", (unsigned int)8888);
  // This will output the tempString to the S7S
  s7sSendStringI2C(tempString);

  //Build the empty packet
  txPacket[0] = '~';
  txPacket[1] = ' ';
  txPacket[2] = ' ';
  txPacket[3] = ' ';
  txPacket[4] = ' ';
  txPacket[5] = ' ';
  txPacket[6] = ' ';
  txPacket[7] = ' ';
  txPacket[8] = ' ';
  txPacket[9] = ' ';
  txPacket[10] = ' ';
  txPacket[11] = ' ';
  txPacket[12] = ' ';
  txPacket[13] = ' ';
  txPacket[14] = ' ';
  txPacket[15] = ' ';
  txPacket[16] = ' ';
  txPacket[17] = ' ';
  txPacket[18] = ' ';
  txPacket[19] = ' ';
  txPacket[20] = ' ';
  txPacket[21] = ' ';
  txPacket[22] = 0x0D;
  txPacket[23] = 0x0A;
  
}

void loop()
{
//**Copy to make a new timer******************//  
//    msTimerA.update(msTicks);
	 debugTimerClass.update(msTicks);
	 serialReadCheckTimer.update(msTicks);
	 ledToggleTimer.update(msTicks);
	 panelUpdateTimer.update(msTicks);
	 debounceTimer.update(msTicks);
	 
//**Copy to make a new timer******************//  
//  if(msTimerA.flagStatus() == PENDING)
//  {
//    digitalWrite( LEDPIN, digitalRead(LEDPIN) ^ 1 );
//  }
//**Debounce timer****************************//  
 if(debounceTimer.flagStatus() == PENDING)
 {
   myPanel.timersMIncrement(5);
   
 }
//**Update the panel LEDs and stuff***********//  
 if(panelUpdateTimer.flagStatus() == PENDING)
 {
   myPanel.update();
   //Check for new data
   	uint8_t tempStatus = 0;
	tempStatus |= myPanel.hazardsSwitch.newData;
	tempStatus |= myPanel.matchReady.newData;
	tempStatus |= myPanel.matchPause.newData;
	tempStatus |= myPanel.countStart.newData;
	tempStatus |= myPanel.countReset.newData;
	tempStatus |= myPanel.eStop.newData;
	tempStatus |= myPanel.doorsSwitch.newData;
	// If new, ship it!
	
	if( tempStatus )
	{
		uint8_t tempValue = 0;
		tempValue |= (myPanel.hazardsSwitch.getState() << 3);
		tempValue |= (myPanel.doorsSwitch.getState() << 2);
		txPacket[7] = hex2char(tempValue);
		
		tempValue = 0;
		tempValue |= (myPanel.matchReady.getState() << 3);
		tempValue |= (myPanel.matchPause.getState() << 2);
		tempValue |= (myPanel.countStart.getState() << 1);
		tempValue |= (myPanel.countReset.getState());
		txPacket[5] = hex2char(tempValue);
		
		tempValue = 0;
		tempValue |= (myPanel.eStop.getState() << 3);
		txPacket[6] = hex2char(tempValue);
		
		txPacket[8] = '0';
		
		for(int i = 0; i < PACKET_LENGTH; i++)
		{
			Serial.write(txPacket[i]);
		}
		Serial.write(0x0A);
	}
	
	// if( tempStatus )
	// {
		// Serial.println("Changed.");
		
	// }
	// delay(10);
	// myPanel.update();
	
 }
 if(debugTimerClass.flagStatus() == PENDING)
 {
   // if( debugLedStates == 0 )
   // {
	   // debugLedStates = 1;
       // //Set all LED off
       // myPanel.hazardsOffLed.setState(LEDOFF);
       // myPanel.hazardsActiveLed.setState(LEDOFF);
       // //myPanel.matchReadyLed.setState(LEDOFF);
       // myPanel.matchPauseLed.setState(LEDOFF);
       // myPanel.doorsUnlockLed.setState(LEDOFF);
       // myPanel.doorsLeftAjarLed.setState(LEDOFF);
       // myPanel.doorsRightAjarLed.setState(LEDOFF);
       // //myPanel.update();	   
   // }
   // else
   // {
	   // debugLedStates = 0;
	   // //Set all LED on
       // myPanel.hazardsOffLed.setState(LEDON);
       // myPanel.hazardsActiveLed.setState(LEDON);
       // //myPanel.matchReadyLed.setState(LEDON);
       // myPanel.matchPauseLed.setState(LEDON);
       // myPanel.doorsUnlockLed.setState(LEDON);
       // myPanel.doorsLeftAjarLed.setState(LEDON);
       // myPanel.doorsRightAjarLed.setState(LEDON);
       // //myPanel.update();   
   // }
  }
  //**LED toggling of the panel class***********//  
 if(ledToggleTimer.flagStatus() == PENDING)
 {
   ledToggleState = ledToggleState ^ 0x01;
   
 }
  //**Ckeck serial input buffer*****************//  
  if(serialReadCheckTimer.flagStatus() == PENDING)
  {
    if (Serial.available())
    {
      lastchar = Serial.read();
      //look for packet start (START_SYMBOL)
      if( lastchar == START_SYMBOL )
      {
		  //Serial.println("START SYMBOL");
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
		  //Serial.print("DATA: ");
        //put the char in the packet
        rxPacket[packet_ptr] = lastchar;
		//Serial.println(rxPacket[packet_ptr]);
		//rxPacket[packet_ptr] = char2hex(rxPacket[packet_ptr]);
        //advance the pointer
        packet_ptr++;
        //turn on LED
      }
      else
      {
		  //Serial.println("END");
        //Just overwrite to the last position
        rxPacket[PACKET_LENGTH - 1] = lastchar;
      }
    }
    
    //if the packet is full and the last char is LF or CR, *do something here*
    // if((packetPending == 1) && ((packet_ptr == (PACKET_LENGTH)) && ((rxPacket[PACKET_LENGTH - 1] == 0x0A) || (rxPacket[PACKET_LENGTH - 1] == 0x0D))) )  // OLD STATEMENT
	if((packetPending == 1) && ((packet_ptr == (PACKET_LENGTH)) && ((rxPacket[PACKET_LENGTH - 1] == 0x0A) )))
    {
		//Serial.println("SERVICING PENDING");
		packetPending = 0;
	  //Convert ascii type information
	  rxPacket[1] = char2hex(rxPacket[1]);
	  rxPacket[2] = char2hex(rxPacket[2]);
	  rxPacket[3] = char2hex(rxPacket[3]);
	  rxPacket[4] = char2hex(rxPacket[4]);
      //Look for LED state
	  switch( (rxPacket[1] >> 2) & 0x03 )
	  {
		  case 0x01:
		  myPanel.hazardsOffLed.setState(LEDON);
		  break;
		  default:
		  case 0x00:
		  myPanel.hazardsOffLed.setState(LEDOFF);
		  break;
		  case 0x02:
		  myPanel.hazardsOffLed.setState(LEDFLASHING);
		  break;
	  }
	  switch( (rxPacket[1]) & 0x03 )
	  {
		  case 0x01:
		  myPanel.hazardsActiveLed.setState(LEDON);
		  break;
		  default:
		  case 0x00:
		  myPanel.hazardsActiveLed.setState(LEDOFF);
		  break;
		  case 0x02:
		  myPanel.hazardsActiveLed.setState(LEDFLASHING);
		  break;
	  }
	  switch( (rxPacket[2] >> 2) & 0x03 )
	  {
		  case 0x01:
		  myPanel.matchReadyLed.setState(LEDON);
		  break;
		  default:
		  case 0x00:
		  myPanel.matchReadyLed.setState(LEDOFF);
		  break;
		  case 0x02:
		  myPanel.matchReadyLed.setState(LEDFLASHING);
		  break;
	  }
	  switch( (rxPacket[2]) & 0x03 )
	  {
		  case 0x01:
		  myPanel.matchPauseLed.setState(LEDON);
		  break;
		  default:
		  case 0x00:
		  myPanel.matchPauseLed.setState(LEDOFF);
		  break;
		  case 0x02:
		  myPanel.matchPauseLed.setState(LEDFLASHING);
		  break;
	  }
	  switch( (rxPacket[3] >> 2) & 0x03 )
	  {
		  case 0x01:
		  myPanel.doorsLeftAjarLed.setState(LEDON);
		  break;
		  default:
		  case 0x00:
		  myPanel.doorsLeftAjarLed.setState(LEDOFF);
		  break;
		  case 0x02:
		  myPanel.doorsLeftAjarLed.setState(LEDFLASHING);
		  break;
	  }
	  switch( (rxPacket[3]) & 0x03 )
	  {
		  case 0x01:
		  myPanel.doorsRightAjarLed.setState(LEDON);
		  break;
		  default:
		  case 0x00:
		  myPanel.doorsRightAjarLed.setState(LEDOFF);
		  break;
		  case 0x02:
		  myPanel.doorsRightAjarLed.setState(LEDFLASHING);
		  break;
	  }
	  switch( (rxPacket[4] >> 2) & 0x03 )
	  {
		  case 0x01:
		  myPanel.doorsUnlockLed.setState(LEDON);
		  break;
		  default:
		  case 0x00:
		  myPanel.doorsUnlockLed.setState(LEDOFF);
		  break;
		  case 0x02:
		  myPanel.doorsUnlockLed.setState(LEDFLASHING);
		  break;
	  }
	  switch( (rxPacket[4]) & 0x03 )
	  {
		  default:
		  break;

	  }
	  tempString[0] = rxPacket[12];
	  tempString[1] = rxPacket[13];
	  tempString[2] = rxPacket[14];
	  tempString[3] = rxPacket[15];
	  
      // This will output the tempString to the S7S
      s7sSendStringI2C(tempString);
	  
	  setDecimalsI2C((char2hex(rxPacket[10]) << 4) | (char2hex(rxPacket[11])));
	  
    }
  }//End checking-for-serial timer
}

ISR(TIMER1_COMPA_vect)
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

// This custom function works somewhat like a serial.print.
//  You can send it an array of chars (string) and it'll print
//  the first 4 characters in the array.
void s7sSendStringI2C(String toSend)
{
  Wire.beginTransmission(s7sAddress);
  for (int i=0; i<4; i++)
  {
    Wire.write(toSend[i]);
  }
  Wire.endTransmission();
}

// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void clearDisplayI2C()
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x76);  // Clear display command
  Wire.endTransmission();
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightnessI2C(byte value)
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x7A);  // Set brightness command byte
  Wire.write(value);  // brightness data byte
  Wire.endTransmission();
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void setDecimalsI2C(byte decimals)
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x77);
  Wire.write(decimals);
  Wire.endTransmission();
}

  



