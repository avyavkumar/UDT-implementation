#include "controlpacket.h"
#include <cstring>
#include <cstdlib>
#include <bitset>

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
m_controlInfo(NULL),
m_packet(NULL)
{
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

char* ControlPacket::makePacket(char* ePayload, int size = 0)
{
  if (!m_type || !m_extendedtype || !m_timestamp || !m_subsequence || !m_controlInfo)
    return NULL;

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
  m_packet = tempo_1 | tempo_2 | tempo_3 | tempo_4;
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
              return Shutdown;
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
    m_controlinfo = (uint32_t *)malloc(sizeof(uint32_t));
    *m_controlinfo = *controlinfo;
    return 1;
  }
  else
    return -1;
}
