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

/****************************************************************************/
/*                          HandShakePacket()                               */
/*                            Constructor                                   */
/****************************************************************************/

HandShakePacket::HandShakePacket():
m_Version(0),
m_Type(0),
m_ISN(0),
m_MSS(0),
m_FlightFlagSize(0),
m_ReqType(0),
m_ID(0)
{
   for (int i = 0; i < 4; ++ i)
      m_piPeerIP[i] = 0;
}

/****************************************************************************/
/*                                 pack()                                   */
/*              Packs the attributes of the handshaking packet              */
/****************************************************************************/

int HandShakePacket::pack(char* buf, int& size)
{
  if (!buf)
    return -1;

  int32_t* p = (int32_t*)buf;
  *p++ = m_Version;
  *p++ = m_Type;
  *p++ = m_ISN;
  *p++ = m_MSS;
  *p++ = m_FlightFlagSize;
  *p++ = m_ReqType;
  *p++ = m_ID;
  for (int i = 0; i < 4; ++ i)
    *p++ = m_piPeerIP[i];

  size = (32/8)*12;
  return 1;
}

/****************************************************************************/
/*                                unpack()                                  */
/*              Unacks the attributes of the handshaking packet             */
/****************************************************************************/

int HandShakePacket::unpack(const char* buf, int size)
{
  if (!buf)
    return -1;

  int32_t* p = (int32_t*)buf;
  m_Version = *p++;
  m_Type = *p++;
  m_ISN = *p++;
  m_MSS = *p++;
  m_FlightFlagSize = *p++;
  m_ReqType = *p++;
  m_ID = *p++;
  for (int i = 0; i < 4; ++ i)
    m_piPeerIP[i] = *p++;

  return 1;
}

int main(int argc, char const *argv[])
{
  HandShakePacket *packet = new HandShakePacket();
  packet->m_Version = 48;
  packet->m_Type = 128;
  packet->m_ISN = 123;
  packet->m_MSS = 213;
  packet->m_FlightFlagSize = 12;
  packet->m_ReqType = 1;
  packet->m_ID = 213;
  for (int i = 0; i < 4; ++ i)
    packet->m_piPeerIP[i] = i;
  char *buffer = (char *)malloc(20*sizeof(char));
  int size = 0;
  packet->pack(buffer, size);
  // packet->unpack(buffer, size);
  // std::cout << packet->m_Version << std::endl;
  // std::cout << packet->m_Type << std::endl;
  // std::cout << packet->m_ISN << std::endl;
  // std::cout << packet->m_MSS << std::endl;
  // std::cout << packet->m_FlightFlagSize << std::endl;
  // std::cout << packet->m_ReqType << std::endl;
  // std::cout << packet->m_ID << std::endl;
  for (int i = 0; i < size; i++)
  {
    std::cout << (int32_t)buffer[i] << std::endl;
  }
  return 0;
}
