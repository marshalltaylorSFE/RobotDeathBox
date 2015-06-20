#include "PanelComponents.h"
//#include "timeKeeper.h"
#include "Arduino.h"
extern uint8_t ledToggleState;

//---Switch------------------------------------------------------
PanelSwitch::PanelSwitch( void )
{

}

void PanelSwitch::init( uint8_t pinNum )
{
  pinNumber = pinNum;
  pinMode( pinNumber, INPUT_PULLUP );
  update();
  //force newData high in case knob starts on last value
  newData = 1;
}

void PanelSwitch::update( void )
{
  uint8_t tempState = digitalRead( pinNumber ) ^ 0x01;
  if( state != tempState )
  {
    state = tempState;
    newData = 1;
  }
}

uint8_t PanelSwitch::getState( void )
{
  newData = 0;

  return state;
}

//---Button------------------------------------------------------
PanelButton::PanelButton( void )
{

}

void PanelButton::init( uint8_t pinNum )
{
  pinNumber = pinNum;
  pinMode( pinNumber, INPUT );
  update();
  //force newData high in case knob starts on last value
  newData = 1;
}

void PanelButton::update( void )
{
	//Serial.println(digitalRead( pinNumber ));
  uint8_t tempState = digitalRead( pinNumber );
  if(( state != tempState ) && (buttonDebounceTimeKeeper.mGet() > 1000))
  {
	//Serial.println(buttonDebounceTimeKeeper.mGet());
    state = tempState;
    newData = 1;
	//Start the timer
	buttonDebounceTimeKeeper.mClear();
  }
}

uint8_t PanelButton::getState( void )
{
  newData = 0;

  return state;
}
//---Led---------------------------------------------------------
PanelLed::PanelLed( void )
{
  state = LEDOFF;
  
}

void PanelLed::init( uint8_t pinNum )
{
  pinNumber = pinNum;
  pinMode( pinNumber, OUTPUT );
  update();

}

void PanelLed::update( void )
{
	switch(state)
	{
		case LEDON:
		digitalWrite( pinNumber, 0 );
		break;
		case LEDFLASHING:
		digitalWrite( pinNumber, ledToggleState);
		break;
		default:
		case LEDOFF:
		digitalWrite( pinNumber, 1 );
		break;
	}

}

ledState_t PanelLed::getState( void )
{
  return state;
  
}

void PanelLed::setState( ledState_t inputValue )
{
  state = inputValue;
  
}
