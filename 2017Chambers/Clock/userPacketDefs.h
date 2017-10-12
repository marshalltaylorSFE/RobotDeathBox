//This file specifies the packets used by the application.
//These are structures the must be transmitted and recieved
//As a stream of bytes somehow.
#ifndef USERPACKETDEFS_H
#define USERPACKETDEFS_H
struct arenaPacket
{
	uint8_t packetStatus;
	uint8_t byteArray[8];
};

#endif