#include "PanelComponents.h"

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

//---Switch------------------------------------------------------
PanelButton::PanelButton( void )
{

}

void PanelButton::init( uint8_t pinNum )
{
  pinNumber = pinNum;
  pinMode( pinNumber, INPUT_PULLUP );
  update();
  //force newData high in case knob starts on last value
  newData = 1;
}

void PanelButton::update( void )
{
  uint8_t tempState = digitalRead( pinNumber ) ^ 0x01;
  if( state != tempState )
  {
    state = tempState;
    newData = 1;
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