#include "Panel.h"
//#include "timeKeeper.h"

#define hazardsSwitchPin A2
#define hazardsOffLedPin 6
#define hazardsActiveLedPin 7
#define matchReadyPin 13
#define matchPausePin 12
#define matchReadyLedPin 5
#define matchPauseLedPin 4
#define countStartPin A3
#define countResetPin 11
#define eStopPin A1
#define doorsSwitchPin A0
#define doorsUnlockLedPin 8
#define doorsLeftAjarLedPin 9
#define doorsRightAjarLedPin 10

Panel::Panel( void )
{

}

void Panel::init( void )
{
  hazardsSwitch.init(hazardsSwitchPin);
  hazardsOffLed.init(hazardsOffLedPin);
  hazardsActiveLed.init(hazardsActiveLedPin);
  matchReady.init(matchReadyPin);
  matchPause.init(matchPausePin);
  matchReadyLed.init(matchReadyLedPin);
  matchPauseLed.init(matchPauseLedPin);
  countStart.init(countStartPin);
  countReset.init(countResetPin);
  eStop.init(eStopPin);
  doorsSwitch.init(doorsSwitchPin);
  doorsUnlockLed.init(doorsUnlockLedPin);
  doorsLeftAjarLed.init(doorsLeftAjarLedPin);
  doorsRightAjarLed.init(doorsRightAjarLedPin);
  
}

void Panel::update( void )
{
  hazardsSwitch.update();
  hazardsOffLed.update();
  hazardsActiveLed.update();
  matchReady.update();
  matchPause.update();
  matchReadyLed.update();
  matchPauseLed.update();
  countStart.update();
  countReset.update();
  eStop.update();
  doorsSwitch.update();
  doorsUnlockLed.update();
  doorsLeftAjarLed.update();
  doorsRightAjarLed.update();
}

void Panel::timersMIncrement( uint8_t inputValue )
{
  matchReady.buttonDebounceTimeKeeper.mIncrement(inputValue);
  matchPause.buttonDebounceTimeKeeper.mIncrement(inputValue);
  countStart.buttonDebounceTimeKeeper.mIncrement(inputValue);
  countReset.buttonDebounceTimeKeeper.mIncrement(inputValue);
  eStop.buttonDebounceTimeKeeper.mIncrement(inputValue);
  
}
