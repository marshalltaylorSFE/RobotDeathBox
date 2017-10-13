//License information:
//Haha!  You use this code.  Use it!

#include "Arduino.h"
#include "uCPacketClass.h"
#include "HOS_char.h"

#define DEFAULT_RX_BURST_SIZE 3

uCPacketUART::uCPacketUART( HardwareSerial * inSerial, uint16_t requestedBufferSize )
{
	rxBufferIndex = 0;
	txBufferIndex = 0;
	
	txPacketInProgress = 0;
	rxPacketPendingSize = 0;
	
	//Configure packets
	startSymbol = '~';
	tail[0] = 0x0D;
	tail[1] = 0x0A;
	tail[2] = 0x00;
  
	recordingPacket = 0;
	
	//Save the size
	bytesAllocated = requestedBufferSize;
	rxBuffer = new uint8_t[bytesAllocated];
	txBuffer = new uint8_t[bytesAllocated];
	linkSerial = inSerial;
};

uCPacketUART::~uCPacketUART( void )
{
	//Trash it
	delete[] rxBuffer;
	delete[] txBuffer;
	bytesAllocated = 0;
};

void uCPacketUART::burstReadInputBuffer( void )
{
	if( rxPacketPendingSize == 0 ) //the previous packet was dumped
	//store characters from input
	{
		int spinning = 1;
		while( spinning )
		{
			if (linkSerial->available() )
			{
				spinning++;
				char lastchar = linkSerial->read();
				//look for packet start (startSymbol)
				if( lastchar == startSymbol )
				{
					rxBufferIndex = 0;
					recordingPacket = 1;
					//Fill packet with null, reset the pointer
					//for( int i = 0; i < PACKET_LENGTH; i++ )
					//{
					//packet[i] = 0;
					//}
					//write the start char
				}
				else if((recordingPacket == 1) && ((lastchar == 0x0A) || (lastchar == 0x0D)) )
				{
					//if the packet is being recorded and the last char is LF or CR, *do something here*
					recordingPacket = 0;
					rxPacketPendingSize = rxBufferIndex;
					//end hit, get out
					spinning = 0;
				}
				else if( ( rxBufferIndex < bytesAllocated ) && ( recordingPacket ) )//check for room in the packet, and that the start char has been seen
				{
					//put the char in the packet
					rxBuffer[rxBufferIndex] = lastchar;
					//advance the pointer
					rxBufferIndex++;
					//turn on LED
				}
				else
				{
					//dump the data
					
				}
			}
			else
			{
				spinning = 0;
			}
			if( spinning > DEFAULT_RX_BURST_SIZE )
			{
				spinning = 0;
			}
		}
	}
};

void uCPacketUART::abandonRxPacket( void )
{
	//Dump a packet that has been flagged
	rxPacketPendingSize = 0;
};

uint16_t uCPacketUART::available( void )
{
	return ( rxPacketPendingSize >> 1 ); //Convert nybble count to byte size
};

void uCPacketUART::getPacket( uint8_t * packetVar, uint16_t sizeVar )
{
	if( rxPacketPendingSize )
	{
		//There is one
		int rxPointer = 0;
		int inputPacketPointer = 0;
		uint8_t tempByte;
		//For each input byte, get two rx nibbles
		while( inputPacketPointer < sizeVar )
		{
			tempByte = char2hex(rxBuffer[rxPointer++]) << 4;
			tempByte |= char2hex(rxBuffer[rxPointer++]);
			packetVar[inputPacketPointer] = tempByte;
			inputPacketPointer++;
		}
		//discard it
		rxPacketPendingSize = 0;
	}
};

uint8_t uCPacketUART::sendPacket( uint8_t * packetVar, uint16_t sizeVar )
{
	if( txPacketInProgress )
	{
		return 1;
	}
	else
	{
		txBufferIndex = 0;
		txBuffer[txBufferIndex++] = startSymbol;
		for( int inputPacketPointer = 0; inputPacketPointer < sizeVar; inputPacketPointer++ )
		{
			txBuffer[txBufferIndex++] = hex2char((packetVar[inputPacketPointer] >> 4) & 0x0F);
			txBuffer[txBufferIndex++] = hex2char((packetVar[inputPacketPointer]) & 0x0F);
		}
		txBuffer[txBufferIndex++] = tail[0];
		txBuffer[txBufferIndex++] = tail[1];
		txBuffer[txBufferIndex++] = tail[2];

		for( int i = 0; txBuffer[i] != 0x00; i++ )
		{
			linkSerial->print((char)txBuffer[i]);
		}
		return 0;
	}
	
};

