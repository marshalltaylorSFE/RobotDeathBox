//Header
#ifndef CUSTOMPANEL_H_INCLUDED
#define CUSTOMPANEL_H_INCLUDED

#include "Panel.h"
#include "PanelComponents.h"
#include "HardwareInterfaces.h"
#include <Arduino.h>

enum PStates
{
	PInit,
	PIdle,
	PWaitForParties,
	PRun
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
	uint8_t timeRemaining = 20;
	uint8_t timerRunning = 1;
	uint8_t timeExpired = 0;
	
private:
	//Internal Panel Components
	Button startButton;
	Button stopButton;
	Button resetButton;
	Button redButton;
	Button blueButton;
	Led redLed;
	Led blueLed;

	//State machine stuff  
	PStates state;
	uint8_t blueState;
	uint8_t redState;

};

#endif