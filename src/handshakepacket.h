#include "packet.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>

class HandShakePacket
{
public:
   HandShakePacket();
   int pack(char* buf, int& size);
   int unpack(const char* buf, int size);
   int32_t m_Version;                             // UDT version
   int32_t m_Type;                                // UDT socket type
   int32_t m_ISN;                                 // random initial sequence number
   int32_t m_MSS;                                 // maximum segment size
   int32_t m_FlightFlagSize;                      // flow control window size
   int32_t m_ReqType;                             // connection request type:
                                                  //  1: regular connection request,
                                                  //  0: rendezvous connection request,
                                                  //  -1/-2: response
   int32_t m_ID;		                              // socket ID
   uint32_t m_piPeerIP[4];	                      // The IP address that the peer's UDP port is bound to
};
