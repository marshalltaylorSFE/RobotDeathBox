//**********************************************************************//
//  BEERWARE LICENSE
//
//  This code is free for any use provided that if you meet the author
//  in person, you buy them a beer.
//
//  This license block is BeerWare itself.
//
//  Written by:  Marshall Taylor
//  Created:  May 26, 2015
//
//**********************************************************************//

#include "arenaStateMachine.h"

//#include "SoftwareSerial.h"
//extern SoftwareSerial Serial;
#include "Arduino.h"
#include "Panel.h"

#include "timeKeeper.h"

#define MATCHLENGTHSECONDS 120
#define STARTHAZARDSSECONDS 61

// enum AStates {
  // AIdle,
  // AWaitForPlayers,
  // AFirstStage,
  // ASecondStage,
  // APauseMatch,
  // ABuzzer,
  // AEnd,
  // ACountOut

// };

ArenaStateMachine::ArenaStateMachine()
{
	//Outputs
	sSDisplay[0] = ' ';
	sSDisplay[1] = ' ';
	sSDisplay[2] = ' ';
	sSDisplay[3] = ' ';
	doorsLocked = 0;
	hazardsOn = 0;
	hazardsOffLed = 0;
	hazardsActiveLed = 0;
	matchReadyLed = 0;
	matchPauseLed = 0;
	doorsUnlockLed = 0;
	doorsLeftAjarLed = 0;
	doorsRightAjarLed = 0;
	matchRunning = 1;
	hammersOn = 0;
	
	//Inputs
	hazardsSwitch = 0;
	judgesReady = 0;
	hammerTimeSwitch = 0;
	playerBlueReady = 1;
	playerRedReady = 1;
	matchPause = 0;
	countStart = 0;
	countReset = 0;
	eStop = 0;
	doorsSwitch = 0;
	doorsLeftAjar = 0;
	doorsRightAjar = 0;
	
	//matchSecondsRemaining = 300; Not needed?
	
	//Player buttons
	redSquareReady = 0;
	redSquareTapOut = 0;
	redSquareLeds = 0;
	blueSquareReady = 0;
	blueSquareTapOut = 0;
	blueSquareLeds = 0;
	
	state = AIdle;
}

void ArenaStateMachine::tick()
{
	//***** PROCESS THE LOGIC *****//
	//

	//Tend to the time keeper
	//tickTimer.mIncrement( 2 );
	//tickTimerHoldOff.mIncrement( 2 );

    //Now do the states.
    AStates nextState = state;
    switch( state )
    {
    case AIdle:
		//Serial.println(hazardsSwitch);
		//Serial.println(hammerTimeSwitch);
		setLedAllOff();
		hazardsActiveLed = 0;
		matchPauseLed = 0;
		matchCounter.mSet(MATCHLENGTHSECONDS);
		matchRunning = 0;
		displayMode = 0;
		matchReadyLed = 0;

		if( doorsSwitch == 1 )
		{
			doorsLocked = 0;
			doorsUnlockLed = 2;
		}
		else
		{
			doorsLocked = 1;
			doorsUnlockLed = 0;			
		}
		if(( hazardsSwitch == 1 ) && (hammerTimeSwitch == 1))
		{
			hazardsOffLed = 0;
		}
		else
		{
			hazardsOffLed = 2;
		}
		if((doorsLeftAjar + doorsRightAjar == 0)  && (doorsSwitch == 0))
		{
			//arena is ready
			matchReadyLed = 1;
		}
		else
		{
			//arena not ready
			matchReadyLed = 2;
		}
		//If Judges and arena are ready, move on
		if( ((judgesReady) == 1) && (doorsLeftAjar + doorsRightAjar == 0)  && (doorsSwitch == 0) )
		{
			//Judges are ready, move on
			nextState = AWaitForPlayers;
			Serial.println("moving to AWaitForPlayers");
			matchRunning = 0;
			displayMode = 1;
			matchReadyLed = 0;
			matchPauseLed = 0;
			doorsLocked = 1;
			//Set the players buttons
			redSquareLeds = 1; //green
			blueSquareLeds = 1;
			playerRedReady = 0;
			playerBlueReady = 0;
			clearAllButtons();
			setLedAllWhite();
			//Make it green
			setLedAllGreen();

		}
		else //Reset the button
		{
			judgesReady = 0;
		}
        break;
	case AWaitForPlayers:
	    //Came from AIdle
		if(( hazardsSwitch == 1 ) && (hammerTimeSwitch == 1))
		{
			hazardsOffLed = 0;
		}
		else
		{
			hazardsOffLed = 2;			
		}
		
		//Manage player buttons
		if( redSquareLeds == 1 ) //If green, allow red
		{
			if(redSquareReady == 1)
			//if(1)// ******DEBUG*****
			{
				redSquareReady = 0;
				redSquareLeds = 2;
				playerRedReady = 1;
				setLedRedSide();
			}
		}
		if( blueSquareLeds == 1 ) //If green, allow red
		{
			if(blueSquareReady == 1)
			//if(1)// ******DEBUG*****
			{
				blueSquareReady = 0;
				blueSquareLeds = 2;
				playerBlueReady = 1;
				setLedBlueSide();
			}
		}
		//High priority: If the arena is no longer ready, go back to idle
		//If both players good, do something
		if((doorsLeftAjar + doorsRightAjar != 0) || (doorsSwitch == 1))
		{
			Serial.println("Reject match due to arena, going to AIdle");
			nextState = AIdle;
			clearAllButtons();
		}
		else if( (playerBlueReady + playerRedReady + judgesReady) == 3)
		{
			setLedAllWhite();
			//Players and arena ready
			nextState = AFirstStage;
			Serial.println("moving to AFirstStage");
			matchRunning = 1;
			clearAllButtons();
			displayMode = 1;
			matchReadyLed = 0;
			matchPauseLed = 0;
		}
        break;
	case AFirstStage:
		if( hazardsSwitch == 1 )
		{
			hazardsOffLed = 0;
		}
		else
		{
			hazardsOffLed = 1;			
		}
		if( matchCounter.mGet() < STARTHAZARDSSECONDS )
		{
			if(( hazardsSwitch == 1 )&&(doorsLeftAjar + doorsRightAjar == 0))
			{
				//turn those hazards on
				hazardsOn = 1;
				hazardsActiveLed = 1;
				if( hammerTimeSwitch == 1 )
				{
					//turn those hazards on
					hammersOn = 1;
				}
				else
				{
					hammersOn = 0;
				}
			}
			else
			{
				hazardsOn = 0;
				hammersOn = 0;
				hazardsActiveLed = 2;
			}

		}

		//Timer expired -- priority
		if( matchCounter.mGet() == 0)
		{
			hazardsOn = 0;
			hammersOn = 0;
			hazardsActiveLed = 0;
			//Start safety timer
			nextState = ABuzzer;
			Serial.println("moving to ABuzzer");
			clearAllButtons();
		}
		//Doors popped or pause -- next priority
		else if((doorsSwitch == 1) || (doorsLeftAjar + doorsRightAjar > 0) || ( matchPause == 1 ) )
		{
			judgesReady = 0;
			matchPause = 0;
			hazardsOn = 0;
			hammersOn = 0;
			matchRunning = 0;
			matchReadyLed = 1;
			matchPauseLed = 1;
			nextState = APauseMatch;
			Serial.println("moving to APauseMatch");
			clearAllButtons();
		}
		else if (( matchCounter.mGet() > 11) && (countStart == 1 ))
		{
			//Switch to countout mode
			nextState = ACountOut;
			Serial.println("moving to ACountOut");
			countoutCounter.mSet(10);
			displayMode = 2;
			clearAllButtons();
		}
		break;
	case APauseMatch:
		if( doorsSwitch == 1 )
		{
			doorsLocked = 0;
			doorsUnlockLed = 1;
		}
		else
		{
			doorsLocked = 1;
			doorsUnlockLed = 0;			
		}
		if( hazardsSwitch == 0 )
		{
			hazardsOffLed = 1;
		}
		else
		{
			hazardsOffLed = 0;			
		}
		hazardsOn = 0;
		hammersOn = 0;
		hazardsActiveLed = 0;
		if((doorsLeftAjar + doorsRightAjar == 0)  && (doorsSwitch == 0))
		{
			//arena is ready
			matchReadyLed = 1;
		}
		else
		{
			//arena not ready
			matchReadyLed = 2;
		}
		if(((judgesReady == 1 )||( matchPause == 1)) && (doorsLeftAjar + doorsRightAjar == 0) && (doorsSwitch == 0))
		{
			nextState = AFirstStage;
			Serial.println("moving to AFirstStage from APauseMatch");
			clearAllButtons();
			matchRunning = 1;
			matchPauseLed = 0;
		}
		else
		{
			judgesReady = 0;
			matchPause = 0;
		}
		break;
	case ABuzzer:
		//Start buzzer timer
		//if buzzer timer expired:
		nextState = AEnd;
		Serial.println("moving to AIdle");
		setLedAllRed();
		break;
	case AEnd:
		break;
	case ACountOut:
		hazardsOffLed = hazardsSwitch ^ 0x01;
		if( matchCounter.mGet() < STARTHAZARDSSECONDS )
		{
			if(( hazardsSwitch == 1 )&&(doorsLeftAjar + doorsRightAjar == 0))
			{
				//turn those hazards on
				hazardsOn = 1;
				hazardsActiveLed = 1;
				if( hammerTimeSwitch == 1 )
				{
					//turn those hazards on
					hammersOn = 1;
				}
				else
				{
					hammersOn = 0;
				}
			}
			else
			{
				hazardsOn = 0;
				hammersOn = 0;
				hazardsActiveLed = 2;
			}
		}
		if( countoutCounter.mGet() == 0 )
		{
			//Counted OUT!
			hazardsOn = 0;
			hammersOn = 0;
			hazardsActiveLed = 0;
			nextState = ABuzzer;
			Serial.println("moving to ABuzzer");
		}
		//Doors popped or pause -- next prio rity
		else if((doorsSwitch == 1) || (doorsLeftAjar + doorsRightAjar > 0) || ( matchPause == 1 ) )
		{
			judgesReady = 0;
			matchPause = 0;
			hazardsOn = 0;
			hammersOn = 0;
			matchRunning = 0;
			matchReadyLed = 1;
			matchPauseLed = 2;
			nextState = APauseMatch;
			Serial.println("moving to APauseMatch");
		}
		else if ( countReset == 1 )
		{
			//Reset - go back to AFirstStage
			displayMode = 1;
			countReset = 0;
			countStart = 0;
			nextState = AFirstStage;
			Serial.println("moving to AFirstStage");

		}
		break;
	case ATapOutHold:
		break;
    default:
        nextState = AIdle;
		Serial.println("!!!DEFAULT STATE HIT!!!");
        break;
    }
	//Tap-out states
	if( blueSquareTapOut == 1 )
	{
		setLedAllRed();
		blueSquareLeds = 0;
		matchRunning = 0;
		blueSquareTapOut = 0;
		redSquareTapOut = 0;
		nextState = ATapOutHold;
		hazardsOn = 0;
		hammersOn = 0;
		hazardsActiveLed = 0;
		Serial.println("BSTO - moving to ATapOutHold");
	}
	if( redSquareTapOut == 1 )
	{
		setLedAllRed();
		redSquareLeds = 0;
		matchRunning = 0;
		blueSquareTapOut = 0;
		redSquareTapOut = 0;
		nextState = ATapOutHold;
		hazardsOn = 0;
		hammersOn = 0;
		hazardsActiveLed = 0;
		Serial.println("RSTO - moving to ATapOutHold");
	}
	//Master with the E-Stop
	if( eStop == 1 )
	{
		Serial.println("E-Stop hit, going to AIdle");
		eStop = 0;
		hazardsOn = 0;
		hammersOn = 0;
		nextState = AIdle;
	}
    state = nextState;

}

void ArenaStateMachine::clearAllButtons( void )
{
		redSquareReady = 0;
		blueSquareReady = 0;
		redSquareTapOut = 0;
		blueSquareTapOut = 0;
		matchPause = 0;
		judgesReady = 0;
		countStart = 0;
		countReset = 0;
		eStop = 0;
		
}

void ArenaStateMachine::setLedAllWhite( void )
{
	digitalWrite(BLUESQUARE_RED, 1);
	digitalWrite(BLUESQUARE_GREEN, 1);
	digitalWrite(BLUESQUARE_BLUE, 1);
	
	digitalWrite(REDSQUARE_RED, 1);
	digitalWrite(REDSQUARE_GREEN, 1);
	digitalWrite(REDSQUARE_BLUE, 1);
	
	digitalWrite(MAINDOOR_RED, 1);
	digitalWrite(MAINDOOR_GREEN, 1);
	digitalWrite(MAINDOOR_BLUE, 1);
	
	digitalWrite(REARDOOR_RED, 1);
	digitalWrite(REARDOOR_GREEN, 1);
	digitalWrite(REARDOOR_BLUE, 1);
}

void ArenaStateMachine::setLedAllRed( void )
{
	digitalWrite(BLUESQUARE_RED, 1);
	digitalWrite(BLUESQUARE_GREEN, 0);
	digitalWrite(BLUESQUARE_BLUE, 0);
	
	digitalWrite(REDSQUARE_RED, 1);
	digitalWrite(REDSQUARE_GREEN, 0);
	digitalWrite(REDSQUARE_BLUE, 0);
	
	digitalWrite(MAINDOOR_RED, 1);
	digitalWrite(MAINDOOR_GREEN, 0);
	digitalWrite(MAINDOOR_BLUE, 0);
	
	digitalWrite(REARDOOR_RED, 1);
	digitalWrite(REARDOOR_GREEN, 0);
	digitalWrite(REARDOOR_BLUE, 0);
}

void ArenaStateMachine::setLedAllGreen( void )
{
	digitalWrite(BLUESQUARE_RED, 0);
	digitalWrite(BLUESQUARE_GREEN, 1);
	digitalWrite(BLUESQUARE_BLUE, 0);
	
	digitalWrite(REDSQUARE_RED, 0);
	digitalWrite(REDSQUARE_GREEN, 1);
	digitalWrite(REDSQUARE_BLUE, 0);
	
	digitalWrite(MAINDOOR_RED, 0);
	digitalWrite(MAINDOOR_GREEN, 1);
	digitalWrite(MAINDOOR_BLUE, 0);
	
	digitalWrite(REARDOOR_RED, 0);
	digitalWrite(REARDOOR_GREEN, 1);
	digitalWrite(REARDOOR_BLUE, 0);
}

void ArenaStateMachine::setLedAllBlue( void )
{
	digitalWrite(BLUESQUARE_RED, 0);
	digitalWrite(BLUESQUARE_GREEN, 0);
	digitalWrite(BLUESQUARE_BLUE, 1);
	
	digitalWrite(REDSQUARE_RED, 0);
	digitalWrite(REDSQUARE_GREEN, 0);
	digitalWrite(REDSQUARE_BLUE, 1);
	
	digitalWrite(MAINDOOR_RED, 0);
	digitalWrite(MAINDOOR_GREEN, 0);
	digitalWrite(MAINDOOR_BLUE, 1);
	
	digitalWrite(REARDOOR_RED, 0);
	digitalWrite(REARDOOR_GREEN, 0);
	digitalWrite(REARDOOR_BLUE, 1);
}

void ArenaStateMachine::setLedAllOff( void )
{
	digitalWrite(BLUESQUARE_RED, 0);
	digitalWrite(BLUESQUARE_GREEN, 0);
	digitalWrite(BLUESQUARE_BLUE, 0);
	
	digitalWrite(REDSQUARE_RED, 0);
	digitalWrite(REDSQUARE_GREEN, 0);
	digitalWrite(REDSQUARE_BLUE, 0);
	
	digitalWrite(MAINDOOR_RED, 0);
	digitalWrite(MAINDOOR_GREEN, 0);
	digitalWrite(MAINDOOR_BLUE, 0);
	
	digitalWrite(REARDOOR_RED, 0);
	digitalWrite(REARDOOR_GREEN, 0);
	digitalWrite(REARDOOR_BLUE, 0);
}

void ArenaStateMachine::setLedBlueSide( void )
{
//	digitalWrite(BLUESQUARE_RED, 1);
//	digitalWrite(BLUESQUARE_GREEN, 0);
//	digitalWrite(BLUESQUARE_BLUE, 0);
}

void ArenaStateMachine::setLedRedSide( void )
{
//	digitalWrite(REDSQUARE_RED, 0);
//	digitalWrite(REDSQUARE_GREEN, 0);
//	digitalWrite(REDSQUARE_BLUE, 1);
}