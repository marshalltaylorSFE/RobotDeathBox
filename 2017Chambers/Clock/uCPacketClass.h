//License information:
//Haha!  You use this code.  Use it!
#ifndef UCPACKETCLASS_H
#define UCPACKETCLASS_H

#include "Arduino.h"

class uCPacketUART
{
public:
	uCPacketUART( HardwareSerial *, uint16_t ); //( uint16_t bytesAllocated )
	~uCPacketUART();
//	void initialize( void );
//	void write( uint8_t, uint8_t ); //address, value.
//	uint8_t read( uint8_t );//address
	void burstReadInputBuffer( void );
	void abandonRxPacket( void );
	uint16_t available( void ); //Returns size of packet available
	void getPacket( uint8_t *, uint16_t ); //give address and size
	uint8_t sendPacket( uint8_t *, uint16_t );// Write entire packet to port
//	void queuePacket( ptr, size );
//  uint8_t sendQueueBurst( size ); //Returns remaining bytes or at least empty state
	
//	uint8_t gamepadButtons;
	uint16_t bytesAllocated; //Indication of how much ram has been allocated
	uint8_t * rxBuffer;
	uint8_t * txBuffer;
	uint16_t rxBufferIndex;
	uint16_t txBufferIndex;
	uint16_t rxPacketPendingSize; // equals 0 for not pending
	uint8_t txPacketInProgress;
	uint8_t recordingPacket;
	private:
	HardwareSerial * linkSerial;
	char startSymbol;
	char tail[3];

	
};
#endif