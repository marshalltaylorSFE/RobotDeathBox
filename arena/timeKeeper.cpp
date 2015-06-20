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

//Includes
#include "timeKeeper.h"

//**********************************************************************//
//
//  Time Keeper
//    Keeps time in number of milliseconds
//
//**********************************************************************//
TimeKeeper::TimeKeeper( void )
{
    timeElapsed = 0;

}

void TimeKeeper::mClear( void )
{
    timeElapsed = 0;

}

uint16_t TimeKeeper::mGet( void )
{
    return timeElapsed;
}

void TimeKeeper::mSet( uint16_t setValue )
{
    timeElapsed = setValue;
}

void TimeKeeper::mIncrement( uint8_t varIncrement )
{
    //clamp it.
	if( timeElapsed < MAXTIMER )
	{
		timeElapsed += varIncrement;
	}
}

void TimeKeeper::mDecrement( uint8_t varDecrement )
{
    //clamp it.
	if(((timeElapsed - varDecrement ) >= 0 ) && (timeElapsed != 0 ) )
	{
		timeElapsed -= varDecrement;
	}
}