// #include "packet.h"
// #include "controlpacket.h"
#include <cstring>
#include <cstdlib>
#include <bitset>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>

// types of packets
enum PacketType {DATA = 0x0, CONTROL = 0x8, ERROR};

// types of subcategories in control packets
enum ControlPacketType {HANDSHAKE, ACK, ACK2, NAK, KeepAlive, ShutDown, DropRequest, ERR, UNKNOWN, CONGESTION};

class ControlPacket
{
private:
  std::bitset <128> packet;
  char *m_packet;

protected:
  uint32_t *m_type;                           // type
  uint32_t *m_extendedtype;                   // extended type
  uint32_t *m_subsequence;                    // sub-sequence number
  uint32_t *m_timestamp;                      // timestamp information
  uint32_t *m_controlInfo;                    // control information
  uint32_t layers[4];

public:
  ControlPacket();
  ~ControlPacket();

  int makePacket(char *final_packet);
  PacketType getFlag();
  ControlPacketType getType();
  int setType(uint32_t *type);
  int setExtendedType(uint32_t *message);
  int setSubsequence(uint32_t *funcField);
  int setTimestamp(uint32_t *timestamp);
  int setControlInfo(uint32_t *orderBit);
};

// CONTROL PACKET FORMAT

/****************************************************************************/
/*                                                                          */
/*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1       */
/*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      */
/*   |1|            Type             |        Extended Type          |      */
/*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      */
/*   |                   ACK Subsequence Number                      |      */
/*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      */
/*   |                          Time Stamp                           |      */
/*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      */
/*   |                     Control Information                       |      */
/*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      */
/*                                                                          */
/****************************************************************************/


/****************************************************************************/
/*                           ControlPacket()                                */
/*                             Constructor                                  */
/****************************************************************************/

ControlPacket::ControlPacket():
m_type(NULL),
m_extendedtype(NULL),
m_timestamp(NULL),
m_subsequence(NULL),
m_controlInfo(NULL)
{
  m_packet = (char *)malloc(16*sizeof(char));
  for (int i = 0; i < 4; i++)
    layers[i] = 0;
}

/****************************************************************************/
/*                           ControlPacket()                                   */
/*                            Destructor                                    */
/****************************************************************************/

ControlPacket::~ControlPacket() {}

/****************************************************************************/
/*                              makePacket()                                */
/*                       Construct the control packet                       */
/*           The final packet is returned along with the padded bits        */
/*      All the fields must be defined, otherwise NULL will be returned     */
/****************************************************************************/

int ControlPacket::makePacket(char *final_packet)
{
  if (!m_type || !m_extendedtype || !m_timestamp || !m_subsequence || !m_controlInfo)
    return -1;

  // set the type as data; make the first bit of the packet 1
  // make the next 15 bits the type
  // set the next 16 bits as the extended user defined extended type
  // set the next bit as don't care; the next 31 bits as ACK sub-sequence number
  // set the next 32 bits as timestamp
  // issue the next 32 bits as control information

  // make a temporary string for the first row of the control packet
  uint32_t *temp_1 = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *temp_2 = (uint32_t *)malloc(sizeof(uint32_t));
  *temp_1 = (*m_type << 16) | 0x8000;
  *temp_2 = *m_extendedtype & 0x00FF;
  *temp_1 = *temp_1 | *temp_2;

  layers[0] = *temp_1;
  layers[1] = *m_subsequence;
  layers[2] = *m_timestamp;
  layers[3] = *m_controlInfo;

  free(temp_1);
  free(temp_2);

  std::bitset <128> tempo_1 (layers[0]);
  std::bitset <128> tempo_2 (layers[1]);
  std::bitset <128> tempo_3 (layers[2]);
  std::bitset <128> tempo_4 (layers[3]);

  tempo_1 <<= 96;
  tempo_2 <<= 64;
  tempo_3 <<= 32;
  packet = tempo_1 | tempo_2 | tempo_3 | tempo_4;
  int length = 0;
  for (int i = 0; i < 16; i++)
  {
    std::bitset<8> c;
    int q = i*8;
    for (int j = 0; j < 8; j++)
      c[j] = packet[q++];
    m_packet[length] = char(c.to_ulong());
    length++;
  }
  for (int i = 0; i < length; i++)
    *(final_packet + i) = *(m_packet + i);
  return 1;
}

/****************************************************************************/
/*                              getType()                                   */
/*                   Returns the type of control packet                     */
/****************************************************************************/

ControlPacketType ControlPacket::getType()
{
  switch (*m_type)
  {
    case 2:   //0010 - Acknowledgement (ACK)
              // ACK packet seq. no.
              return ACK;
              break;

    case 6:   //0110 - Acknowledgement of Acknowledgement (ACK-2)
              // ACK packet seq. no.
              return ACK2;
              break;

    case 3:   //0011 - Loss Report (NAK)
              // loss list
              return NAK;
              break;

    case 4:   //0100 - Congestion Warning
              return CONGESTION;
              break;

    case 1:   //0001 - Keep-alive
              return KeepAlive;
              break;

    case 0:   //0000 - Handshake
              // control info filed is handshake info
              return HANDSHAKE;
              break;

    case 5:   //0101 - Shutdown
              return ShutDown;
              break;

    case 7:   //0111 - Message Drop Request
              return DropRequest;
              break;

    case 8:   //1000 - Error Signal from the Peer Side
              return ERR;
              break;

    default:  return UNKNOWN;
              break;
  }
}

/****************************************************************************/
/*                              getFlag()                                   */
/*                        Returns the type of flag                          */
/****************************************************************************/

PacketType ControlPacket::getFlag()
{
  return CONTROL;
}

/****************************************************************************/
/*                                setType()                                 */
/*       Stores the type that should be assigned to the said packet         */
/*         If the parameter sequence is NULL, then -1 is returned           */
/*         Pointer passed can be freed once the assignment is done          */
/*            If the type is set successfully, 1 is returned                */
/****************************************************************************/

int ControlPacket::setType(uint32_t *type)
{
  if (type)
  {
    m_type = (uint32_t *)malloc(sizeof(uint32_t));
    *m_type = *type;
    return 1;
  }
  else
    return -1;
}

/****************************************************************************/
/*                            setExtendedType()                             */
/*       Stores the etype that should be assigned to the said packet        */
/*         If the parameter sequence is NULL, then -1 is returned           */
/*         Pointer passed can be freed once the assignment is done          */
/*            If the etype is set successfully, 1 is returned               */
/****************************************************************************/

int ControlPacket::setExtendedType(uint32_t *extendedtype)
{
  if (extendedtype)
  {
    m_extendedtype = (uint32_t *)malloc(sizeof(uint32_t));
    *m_extendedtype = *extendedtype;
    return 1;
  }
  else
    return -1;
}

/****************************************************************************/
/*                              setTimestamp()                              */
/*   Stores the timestamp that should be assigned to the said packet        */
/*         If the parameter timestamp is NULL, then -1 is returned          */
/*          Pointer passed can be freed once the assignment is done         */
/*          If the timestamp is set successfully, 1 is returned             */
/****************************************************************************/

int ControlPacket::setTimestamp(uint32_t *timestamp)
{
  if (timestamp)
  {
    m_timestamp = (uint32_t *)malloc(sizeof(uint32_t));
    *m_timestamp = *timestamp;
    return 1;
  }
  else
    return -1;
}

/****************************************************************************/
/*                              setSubsequence()                            */
/*     Stores the subsequence that should be assigned to the said packet    */
/*         If the parameter subsequence is NULL, then -1 is returned        */
/*          Pointer passed can be freed once the assignment is done         */
/*          If the subsequence is set successfully, 1 is returned           */
/****************************************************************************/

int ControlPacket::setSubsequence(uint32_t *subsequence)
{
  if (subsequence)
  {
    m_subsequence = (uint32_t *)malloc(sizeof(uint32_t));
    *m_subsequence = *subsequence;
    return 1;
  }
  else
    return -1;
}

/****************************************************************************/
/*                               setControlInfo()                           */
/*     Stores the controlinfo that should be assigned to the said packet    */
/*           If the parameter controlinfo is NULL, then -1 is returned      */
/*          Pointer passed can be freed once the assignment is done         */
/*            If the controlinfo is set successfully, 1 is returned         */
/****************************************************************************/

int ControlPacket::setControlInfo(uint32_t *controlinfo)
{
  if (controlinfo)
  {
    m_controlInfo = (uint32_t *)malloc(sizeof(uint32_t));
    *m_controlInfo = *controlinfo;
    return 1;
  }
  else
    return -1;
}

int main()
{
  ControlPacket *packet = new ControlPacket();
  uint32_t *type = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *etype = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *sub = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *t = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *a = (uint32_t *)malloc(sizeof(uint32_t));
  *type = 1;
  *etype = 0x12;
  *sub = 45;
  *t = 82372;
  *a = 7832;
  packet->setType(type);
  packet->setExtendedType(etype);
  packet->setSubsequence(sub);
  packet->setControlInfo(a);
  packet->setTimestamp(t);
  //std::cout << packet->getType() << std::endl;
  char *s_packet = (char *)malloc(16*sizeof(char));
  int temp = packet->makePacket(s_packet);
  for (int i = 0; i < 16; i++)
    std::cout << s_packet[i] << std::endl;
  return 0;
}
