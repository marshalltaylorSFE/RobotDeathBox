#ifndef PANEL_H
#define PANEL_H

#include "PanelComponents.h"
#include <Arduino.h>
//#include "timeKeeper.h"

class Panel
{
public:
  Panel( void );
  void update( void );
  void init( void );
  void timersMIncrement( uint8_t );
  
  PanelSwitch hazardsSwitch;
  PanelLed hazardsOffLed;
  PanelLed hazardsActiveLed;
  PanelButton matchReady;
  PanelButton matchPause;
  PanelLed matchReadyLed;
  PanelLed matchPauseLed;
  PanelButton countStart;
  PanelButton countReset;
  PanelButton eStop;
  PanelSwitch doorsSwitch;
  PanelLed doorsUnlockLed;
  PanelLed doorsLeftAjarLed;
  PanelLed doorsRightAjarLed;

protected:
private:
};

#endif // PANEL_H



