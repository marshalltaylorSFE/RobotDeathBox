//Header
#ifndef CUSTOMPANEL_H_INCLUDED
#define CUSTOMPANEL_H_INCLUDED

#include "Panel.h"
#include "PanelComponents.h"
#include "HardwareInterfaces.h"
#include <Arduino.h>
#include "timeKeeper.h"

enum PStates
{
	PInit,
	PIdle,
	PWaitForParties,
	PStartHorn,
	PRun,
	PPauseHorn,
	PPause,
	PSelectTime,
	PStopHorn,
	PEnd
};

class CPanel : public Panel
{
public:
	CPanel( void );
	void reset( void );
	void tickStateMachine( int msTicksDelta );
	uint8_t getState( void ){
		return (uint8_t)state;
	}
	int16_t timeRemaining;
	int16_t timeSetting = 120;
	uint8_t timerRunning;
	uint8_t timeExpired;
	uint8_t displayOn;
	uint8_t displayFlashing;
	
private:
	//Internal Panel Components
	Button startButton;
	Button stopButton;
	Button resetButton;
	Button redButton;
	Button blueButton;
	Led redLed;
	Led blueLed;
	Led horn;

	//State machine stuff  
	PStates state;
	uint8_t blueState;
	uint8_t redState;
	
	TimeKeeper stateTimer;
};

#endif