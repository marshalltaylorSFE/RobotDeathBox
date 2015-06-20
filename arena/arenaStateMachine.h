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
#ifndef ARENASTATEMACHINE_H_INCLUDED
#define ARENASTATEMACHINE_H_INCLUDED

#include "stdint.h"
#include "timeKeeper.h"

enum AStates {
  AIdle,
  AWaitForPlayers,
  AFirstStage,
  ASecondStage,
  APauseMatch,
  ABuzzer,
  AEnd,
  ACountOut,
  ATapOutHold

};

class ArenaStateMachine
{
public:
  //Outputs
  char sSDisplay[4];
  uint8_t doorsLocked;
  uint8_t hazardsOn;
  uint8_t hazardsOffLed;
  uint8_t hazardsActiveLed;
  uint8_t matchReadyLed;
  uint8_t matchPauseLed;
  uint8_t doorsUnlockLed;
  uint8_t doorsLeftAjarLed;
  uint8_t doorsRightAjarLed;
  uint8_t matchRunning;
  uint8_t displayMode; //Can be 0 for off, 1 for time, 2 for countout
  
  //Inputs
  uint8_t hazardsSwitch;
  uint8_t judgesReady;
  uint8_t playerBlueReady;
  uint8_t playerRedReady;
  uint8_t matchPause;
  uint8_t countStart;
  uint8_t countReset;
  uint8_t eStop;
  uint8_t doorsSwitch;
  uint8_t doorsLeftAjar;
  uint8_t doorsRightAjar;
  
  uint8_t redSquareReady;
  uint8_t redSquareTapOut;
  uint8_t redSquareLeds;
  uint8_t blueSquareReady;
  uint8_t blueSquareTapOut;
  uint8_t blueSquareLeds;

  TimeKeeper matchCounter;
  TimeKeeper countoutCounter;
  
  //State machine stuff  
  AStates state;
  uint8_t matchSecondsRemaining;
  
  ArenaStateMachine( void );
  void tick( void );
  void clearAllButtons( void );
  void setLedAllWhite( void );
  void setLedAllRed( void );
  void setLedAllGreen( void );
  void setLedAllBlue( void );
  void setLedAllOff( void );
  
};

#endif