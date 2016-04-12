#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include "handshakepacket.h"

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
