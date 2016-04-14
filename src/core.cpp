#include "core.h"
// Functionality:
//    initialize a UDT entity and bind to a local address.
// Parameters:
//    None.
// Returned value:
//    None.

int UDTCore::open(UDTSocket *socket)
{

}

// Functionality:
//    Start listening to any connection request.
// Parameters:
//    None.
// Returned value:
//    None.

int UDTCore::listen(UDTSocket *socket)
{
	if (!socket)
  	{
    	std::cerr << "UNAVAILABLE MEMORY FOR SOCKET" << std::endl;
    	return -1;
  	}
  	// intialise HANDSHAKEing information
  	
  	char *buffer = (char *)malloc(MAXSIZE*sizeof(char));
  	//get the first packet
  	if ( (socket->ReceivePacket(buffer)) > 0 ) {	//this is a blocking call
  		// if ( HANDSHAKE packet )
  		// 		send the HANDSHAKEing packet back
  		//		wait for return packet
  		//		return 0;
  		// else
  		// 		discard buffer
  	}
  	return -1;
}
