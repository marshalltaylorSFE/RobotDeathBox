//********************************************//
#include "CPanel.h"
#include "panelComponents.h"
#include "HardwareInterfaces.h"
#include <Arduino.h>

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
	
	state = PInit;
}

void CPanel::reset( void )
{
	//Set explicit states
	//Set all LED off
	redLed.setState(LEDOFF);
	blueLed.setState(LEDOFF);
	state = PInit;
	
}

void CPanel::tickStateMachine( int msTicksDelta )
{
	freshenComponents( msTicksDelta );
	
	//***** PROCESS THE LOGIC *****//
	//Now do the states.
	switch( state )
	{
		case PInit:
		state = PIdle;
		redLed.setState(LEDOFF);
		blueLed.setState(LEDOFF);
		break;
		case PIdle:
		if( startButton.serviceRisingEdge() )
		{
			state = PWaitForParties;
			redLed.setState(LEDFLASHING);
			blueLed.setState(LEDFLASHING);
			blueState = 0;
			redState = 0;
		}
		break;
		case PWaitForParties:
		if(redButton.serviceRisingEdge())
		{
			redLed.setState(LEDON);
			blueState = 1;
		}
		if(blueButton.serviceRisingEdge())
		{
			blueLed.setState(LEDON);
			redState = 1;
		}
		if((blueState == 1)&&(redState == 1))
		{
			state = PRun;
		}
		break;
		case PRun:
		break;
		default:
		break;
	}

}