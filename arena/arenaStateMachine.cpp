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
	
	//Inputs
	hazardsSwitch = 0;
	judgesReady = 0;
	playerLeftReady = 1;
	playerRightReady = 1;
	matchPause = 0;
	countStart = 0;
	countReset = 0;
	eStop = 0;
	doorsSwitch = 0;
	doorsLeftAjar = 0;
	doorsRightAjar = 0;
	
	matchSecondsRemaining = 300;
	
	state = AIdle;
}

void ArenaStateMachine::tick()
{
	//***** PROCESS THE LOGIC *****//

	//Tend to the time keeper
	//tickTimer.mIncrement( 2 );
	//tickTimerHoldOff.mIncrement( 2 );

    //Now do the states.
    AStates nextState = state;
    switch( state )
    {
    case AIdle:
		//If arena is ready
		matchSecondsRemaining = 300;
		doorsLocked = 0;
		if( (doorsLeftAjar + doorsRightAjar == 0) && (doorsSwitch == 0) )
		{
			//Lock doors
			doorsLocked = 1;
			//Move states
			nextState = AWaitForPlayers;
			//Clear ready buttons
			judgesReady = 0;
			playerLeftReady = 1;
			playerRightReady = 1;
		}
        break;
	case AWaitForPlayers:
		if( ((playerLeftReady + playerRightReady + judgesReady) == 3) && (doorsLeftAjar + doorsRightAjar == 0) )
		{
			//Players and arena ready
			nextState = AFirstStage;
		}
		if((doorsSwitch == 1) || (doorsLeftAjar + doorsRightAjar == 0) )
		{
			//Go back to idle
			nextState = AIdle;
		}
        break;
    default:
        nextState = AIdle;
        break;
    }
    state = nextState;

}

