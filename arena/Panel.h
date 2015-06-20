#ifndef PANEL_H
#define PANEL_H

#include "PanelComponents.h"
#include <Arduino.h>
//#include "timeKeeper.h"

#define  BLUESQUARE_RED   37 
#define  BLUESQUARE_GREEN 38
#define  BLUESQUARE_BLUE  39 

#define  REDSQUARE_RED    40
#define  REDSQUARE_GREEN  41
#define  REDSQUARE_BLUE   42 

#define  MAINDOOR_RED    43
#define  MAINDOOR_GREEN  44
#define  MAINDOOR_BLUE   45 
  
#define  REARDOOR_RED    46
#define  REARDOOR_GREEN  47
#define  REARDOOR_BLUE   48


class Panel
{
public:
  Panel( void );
  void update( void );
  void init( void );
  void timersMIncrement( uint8_t );
  
  PanelButton redSquareReady;
  PanelButton redSquareTapOut;
  PanelButton blueSquareReady;
  PanelButton blueSquareTapOut;

protected:
private:
};

#endif // PANEL_H



