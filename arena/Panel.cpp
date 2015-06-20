#include "Panel.h"
//#include "timeKeeper.h"

uint8_t BLUESQUARE_TAPOUT_BUTTON_INPUT = 35;
uint8_t BLUESQUARE_READY_BUTTON_INPUT = 34;

uint8_t REDSQUARE_TAPOUT_BUTTON_INPUT = 23;
uint8_t REDSQUARE_READY_BUTTON_INPUT = 22;

Panel::Panel( void )
{

}

void Panel::init( void )
{
  redSquareReady.init(REDSQUARE_READY_BUTTON_INPUT);
  redSquareTapOut.init(REDSQUARE_TAPOUT_BUTTON_INPUT);
  blueSquareReady.init(BLUESQUARE_READY_BUTTON_INPUT);
  blueSquareTapOut.init(BLUESQUARE_TAPOUT_BUTTON_INPUT);
  
}

void Panel::update( void )
{
  redSquareReady.update();
  redSquareTapOut.update();
  blueSquareReady.update();
  blueSquareTapOut.update();
  
}

void Panel::timersMIncrement( uint8_t inputValue )
{
  redSquareReady.buttonDebounceTimeKeeper.mIncrement(inputValue);
  redSquareTapOut.buttonDebounceTimeKeeper.mIncrement(inputValue);
  blueSquareReady.buttonDebounceTimeKeeper.mIncrement(inputValue);
  blueSquareTapOut.buttonDebounceTimeKeeper.mIncrement(inputValue);
  
}
