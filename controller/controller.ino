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
uint8_t debugLedStates = 0;

// #include "timeKeeper.h"
// TimeKeeper debugTimeKeeper();


//tick variable for interrupt driven timer1
uint16_t msTicks = 0;
uint8_t msTicksMutex = 1; //start locked out


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
  
}

void loop()
{
	uint8_t tempStatus = 0;
	tempStatus |= myPanel.hazardsSwitch.newData;
	tempStatus |= myPanel.matchReady.newData;
	tempStatus |= myPanel.matchPause.newData;
	tempStatus |= myPanel.countStart.newData;
	tempStatus |= myPanel.countReset.newData;
	tempStatus |= myPanel.eStop.newData;
	tempStatus |= myPanel.doorsSwitch.newData;
	
	myPanel.hazardsSwitch.getState();
	myPanel.matchReady.getState();
	myPanel.matchPause.getState();
	myPanel.countStart.getState();
	myPanel.countReset.getState();
	myPanel.eStop.getState();
	myPanel.doorsSwitch.getState();
	
	if( tempStatus )
	{
		Serial.println("Changed.");
		
	}
	delay(10);
	myPanel.update();

//**Copy to make a new timer******************//  
//    msTimerA.update(msTicks);
	 debugTimerClass.update(msTicks);
	 
//**Copy to make a new timer******************//  
//  if(msTimerA.flagStatus() == PENDING)
//  {
//    digitalWrite( LEDPIN, digitalRead(LEDPIN) ^ 1 );
//  }
 if(debugTimerClass.flagStatus() == PENDING)
 {
   if( debugLedStates == 0 )
   {
	   debugLedStates = 1;
       //Set all LED off
       myPanel.hazardsOffLed.setState(LEDOFF);
       myPanel.hazardsActiveLed.setState(LEDOFF);
       myPanel.matchReadyLed.setState(LEDOFF);
       myPanel.matchPauseLed.setState(LEDOFF);
       myPanel.doorsUnlockLed.setState(LEDOFF);
       myPanel.doorsLeftAjarLed.setState(LEDOFF);
       myPanel.doorsRightAjarLed.setState(LEDOFF);
       myPanel.update();	   
   }
   else
   {
	   debugLedStates = 0;
	   //Set all LED on
       myPanel.hazardsOffLed.setState(LEDON);
       myPanel.hazardsActiveLed.setState(LEDON);
       myPanel.matchReadyLed.setState(LEDON);
       myPanel.matchPauseLed.setState(LEDON);
       myPanel.doorsUnlockLed.setState(LEDON);
       myPanel.doorsLeftAjarLed.setState(LEDON);
       myPanel.doorsRightAjarLed.setState(LEDON);
       myPanel.update();   
   }
   
 }	
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












