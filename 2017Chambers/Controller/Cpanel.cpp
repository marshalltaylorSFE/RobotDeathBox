//********************************************//
#include "CPanel.h"
#include "panelComponents.h"
#include "HardwareInterfaces.h"
#include <Arduino.h>
#include "timeKeeper.h"

CPanel::CPanel( void )
{
	startButton.setHardware(new ArduinoDigitalIn( 3 ), 1);
	add( &startButton );
	stopButton.setHardware(new ArduinoDigitalIn( 2 ), 1);
	add( &stopButton );
	resetButton.setHardware(new ArduinoDigitalIn( 4 ), 1);
	add( &resetButton );
	redButton.setHardware(new ArduinoDigitalIn( 8 ), 1);
	add( &redButton );
	blueButton.setHardware(new ArduinoDigitalIn( 10 ), 1);
	add( &blueButton );

	redLed.setHardware(new ArduinoDigitalOut( 9 ), 0);
	add( &redLed );
	blueLed.setHardware(new ArduinoDigitalOut( 11 ), 0);
	add( &blueLed );

	horn.setHardware(new ArduinoDigitalOut( 12 ), 0);
	add( &horn );
	
	state = PInit;
}

void CPanel::reset( void )
{
	//Set explicit states
	//Set all LED off
	redLed.setState(LEDOFF);
	blueLed.setState(LEDOFF);
	horn.setState(LEDOFF);
	state = PInit;
	
}

void CPanel::tickStateMachine( int msTicksDelta )
{
	freshenComponents( msTicksDelta );
	stateTimer.mIncrement(3);
	//***** PROCESS THE LOGIC *****//
	//Now do the states.
	switch( state )
	{
		case PInit:
		state = PIdle;
		redLed.setState(LEDOFF);
		blueLed.setState(LEDOFF);
		horn.setState(LEDOFF);
		timeRemaining = timeSetting;
		timerRunning = 0;
		timeExpired = 0;
		displayOn = 0;
		startButton.serviceRisingEdge();
		stopButton.serviceHoldRisingEdge();
		break;
		case PIdle:
		if( startButton.serviceRisingEdge() )
		{
			state = PWaitForParties;
			redLed.setState(LEDFLASHING);
			blueLed.setState(LEDFLASHING);
			redButton.serviceRisingEdge();
			blueButton.serviceRisingEdge();
			blueState = 0;
			redState = 0;
			displayOn = 1;
			displayFlashing = 1;
		}
		if( stopButton.serviceHoldRisingEdge() )
		{
			startButton.serviceRisingEdge();
			stopButton.serviceRisingEdge();
			state = PSelectTime;
			displayOn = 1;
			displayFlashing = 0;
		}
		break;
		case PWaitForParties:
		if(redButton.serviceRisingEdge())
		{
			redLed.setState(LEDON);
			blueState = 1;
			startButton.serviceRisingEdge();
		}
		if(blueButton.serviceRisingEdge())
		{
			blueLed.setState(LEDON);
			redState = 1;
			startButton.serviceRisingEdge();
		}
		if((blueState == 1)&&(redState == 1))
		{
			displayFlashing = 0;
			if(startButton.serviceRisingEdge())
			{
				horn.setState(LEDON);
				stateTimer.mClear();
				state = PStartHorn;
				stopButton.serviceRisingEdge();
				timerRunning = 1;
			}
		}
		break;
		case PStartHorn:
		if(stateTimer.mGet() > 333)
		{
			horn.setState(LEDOFF);
			state = PRun;
		}
		case PRun:
			if(stopButton.serviceRisingEdge())
			{
				//pausing
				startButton.serviceRisingEdge();
				state = PPauseHorn;
				timerRunning = 0;
				displayFlashing = 1;
				horn.setState(LEDON);
				stateTimer.mClear();
			}
			//Detect end of game
			else if( timerRunning == 0 )
			{
				state = PStopHorn;
				horn.setState(LEDON);
				redLed.setState(LEDOFF);
				blueLed.setState(LEDOFF);
				stateTimer.mClear();
			}
			else if( redButton.serviceRisingEdge() )//player tapped out
			{
				state = PStopHorn;
				horn.setState(LEDON);
				redLed.setState(LEDOFF);
				stateTimer.mClear();
				timerRunning = 0;
			}
			else if( blueButton.serviceRisingEdge() )//player tapped out
			{
				state = PStopHorn;
				horn.setState(LEDON);
				blueLed.setState(LEDOFF);
				stateTimer.mClear();
				timerRunning = 0;
			}
		break;
		case PPauseHorn:
		if(stateTimer.mGet() > 333)
		{
			horn.setState(LEDOFF);
			state = PPause;
		}
		break;
		case PPause:
			if(startButton.serviceRisingEdge())
			{
				stateTimer.mClear();
				state = PStartHorn;
				horn.setState(LEDON);
				timerRunning = 1;
				displayFlashing = 0;
				stopButton.serviceRisingEdge();
				redButton.serviceRisingEdge();
				blueButton.serviceRisingEdge();
			}
		break;
		case PSelectTime:
			if(startButton.serviceRisingEdge())
			{
				if(timeSetting < 1200)
				{
					timeSetting += 30;
				}
			}
			if(stopButton.serviceRisingEdge())
			{
				if(timeSetting > 30 )
				{
					timeSetting -= 30;
				}
			}
			timeRemaining = timeSetting;
		break;
		case PStopHorn:
			if(stateTimer.mGet() > 1500)
			{
				horn.setState(LEDOFF);
				state = PEnd;
			}
		break;
		case PEnd:
			
		break;
		default:
		break;
	}
	if(resetButton.serviceRisingEdge())
	{
		state = PInit;
	}

}