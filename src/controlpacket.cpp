#include "controlpacket.h"

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
/*   |                          Socket ID                            |      */
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
m_socketID(NULL)
{
  m_packet = (char *)malloc(40*sizeof(char));
  for (int i = 0; i < 10; i++)
    layers[i] = 0;
  m_type = (uint32_t *)malloc(sizeof(uint32_t));
  m_extendedtype = (uint32_t *)malloc(sizeof(uint32_t));
  m_timestamp = (uint32_t *)malloc(sizeof(uint32_t));
  m_subsequence = (uint32_t *)malloc(sizeof(uint32_t));
  m_socketID = (uint32_t *)malloc(sizeof(uint32_t));
  m_packetSeq = (uint32_t *)malloc(sizeof(uint32_t));
  m_RTT = (uint32_t *)malloc(sizeof(uint32_t));
  m_RTTVar = (uint32_t *)malloc(sizeof(uint32_t));
  m_availBuffer = (uint32_t *)malloc(sizeof(uint32_t));
  m_packRecvRate = (uint32_t *)malloc(sizeof(uint32_t));
  m_linkCap = (uint32_t *)malloc(sizeof(uint32_t));
  m_Version = (uint32_t *)malloc(sizeof(uint32_t));
  m_Type = (uint32_t *)malloc(sizeof(uint32_t));
  m_ISN = (uint32_t *)malloc(sizeof(uint32_t));
  m_MSS = (uint32_t *)malloc(sizeof(uint32_t));
  m_FlightFlagSize = (uint32_t *)malloc(sizeof(uint32_t));
  m_ReqType = (uint32_t *)malloc(sizeof(uint32_t));
  m_LossInfo = (uint32_t *)malloc(sizeof(uint32_t));
  m_firstMessage = (uint32_t *)malloc(sizeof(uint32_t));
  m_lastMessage = (uint32_t *)malloc(sizeof(uint32_t));
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
  if (!m_type || !m_extendedtype || !m_timestamp || !m_subsequence || !m_socketID)
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
  *temp_1 = (*m_type << 16) | 0x80000000;
  *temp_2 = *m_extendedtype & 0x0000FFFF;
  *temp_1 = *temp_1 | *temp_2;

  layers[0] = *temp_1;
  layers[1] = *m_subsequence;
  layers[2] = *m_timestamp;
  layers[3] = *m_socketID;

  std::bitset <320> tempo_0 (layers[0]);
  std::bitset <320> tempo_1 (layers[1]);
  std::bitset <320> tempo_2 (layers[2]);
  std::bitset <320> tempo_3 (layers[3]);

  free(temp_1);
  free(temp_2);

  if (getPacketType() == ACK)
  {
    layers[4] = *m_packetSeq;
    layers[5] = *m_RTT;
    layers[6] = *m_RTTVar;
    layers[7] = *m_availBuffer;
    layers[8] = *m_packRecvRate;
    layers[9] = *m_linkCap;

    std::bitset <320> tempo_4 (layers[4]);
    std::bitset <320> tempo_5 (layers[5]);
    std::bitset <320> tempo_6 (layers[6]);
    std::bitset <320> tempo_7 (layers[7]);
    std::bitset <320> tempo_8 (layers[8]);
    std::bitset <320> tempo_9 (layers[9]);

    tempo_0 <<= 288;
    tempo_1 <<= 256;
    tempo_2 <<= 224;
    tempo_3 <<= 192;
    tempo_4 <<= 160;
    tempo_5 <<= 128;
    tempo_6 <<= 96;
    tempo_7 <<= 64;
    tempo_8 <<= 32;

    packet = tempo_0 | tempo_1 | tempo_2 | tempo_3 | tempo_4 | tempo_5 | tempo_6 | tempo_7 | tempo_8 | tempo_9;
    int length = 0;
    for (int i = 0; i < 40; i++)
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
    return length;
  }
  else if (getPacketType() == HANDSHAKE)
  {
    layers[4] = *m_Version;
    layers[5] = *m_Type;
    layers[6] = *m_ISN;
    layers[7] = *m_MSS;
    layers[8] = *m_FlightFlagSize;
    layers[9] = *m_ReqType;

    std::bitset <320> tempo_4 (layers[4]);
    std::bitset <320> tempo_5 (layers[5]);
    std::bitset <320> tempo_6 (layers[6]);
    std::bitset <320> tempo_7 (layers[7]);
    std::bitset <320> tempo_8 (layers[8]);
    std::bitset <320> tempo_9 (layers[9]);

    tempo_0 <<= 288;
    tempo_1 <<= 256;
    tempo_2 <<= 224;
    tempo_3 <<= 192;
    tempo_4 <<= 160;
    tempo_5 <<= 128;
    tempo_6 <<= 96;
    tempo_7 <<= 64;
    tempo_8 <<= 32;

    packet = tempo_0 | tempo_1 | tempo_2 | tempo_3 | tempo_4 | tempo_5 | tempo_6 | tempo_7 | tempo_8 | tempo_9;
    int length = 0;
    for (int i = 0; i < 40; i++)
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
    return length;
  }

  else if (getPacketType() == NAK)
  {
    layers[4] = *m_LossInfo;

    std::bitset <320> tempo_4 (layers[4]);
    std::bitset <320> tempo_5 (layers[5]);
    std::bitset <320> tempo_6 (layers[6]);
    std::bitset <320> tempo_7 (layers[7]);
    std::bitset <320> tempo_8 (layers[8]);
    std::bitset <320> tempo_9 (layers[9]);

    tempo_0 <<= 288;
    tempo_1 <<= 256;
    tempo_2 <<= 224;
    tempo_3 <<= 192;
    tempo_4 <<= 160;
    tempo_5 <<= 128;
    tempo_6 <<= 96;
    tempo_7 <<= 64;
    tempo_8 <<= 32;

    packet = tempo_0 | tempo_1 | tempo_2 | tempo_3 | tempo_4 | tempo_5 | tempo_6 | tempo_7 | tempo_8 | tempo_9;

    int length = 0;
    for (int i = 0; i < 40; i++)
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
    return length;
  }

  else if (getPacketType() == DropRequest)
  {
    layers[4] = *m_firstMessage;
    layers[5] = *m_lastMessage;

    std::bitset <320> tempo_4 (layers[4]);
    std::bitset <320> tempo_5 (layers[5]);
    std::bitset <320> tempo_6 (layers[6]);
    std::bitset <320> tempo_7 (layers[7]);
    std::bitset <320> tempo_8 (layers[8]);
    std::bitset <320> tempo_9 (layers[9]);

    tempo_0 <<= 288;
    tempo_1 <<= 256;
    tempo_2 <<= 224;
    tempo_3 <<= 192;
    tempo_4 <<= 160;
    tempo_5 <<= 128;
    tempo_6 <<= 96;
    tempo_7 <<= 64;
    tempo_8 <<= 32;

    packet = tempo_0 | tempo_1 | tempo_2 | tempo_3 | tempo_4 | tempo_5 | tempo_6 | tempo_7 | tempo_8 | tempo_9;

    int length = 0;
    for (int i = 0; i < 40; i++)
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
    return length;
  }
  else
  {

    std::bitset <320> tempo_4 (layers[4]);
    std::bitset <320> tempo_5 (layers[5]);
    std::bitset <320> tempo_6 (layers[6]);
    std::bitset <320> tempo_7 (layers[7]);
    std::bitset <320> tempo_8 (layers[8]);
    std::bitset <320> tempo_9 (layers[9]);

    tempo_0 <<= 288;
    tempo_1 <<= 256;
    tempo_2 <<= 224;
    tempo_3 <<= 192;
    tempo_4 <<= 160;
    tempo_5 <<= 128;
    tempo_6 <<= 96;
    tempo_7 <<= 64;
    tempo_8 <<= 32;

    packet = tempo_0 | tempo_1 | tempo_2 | tempo_3 | tempo_4 | tempo_5 | tempo_6 | tempo_7 | tempo_8 | tempo_9;

    int length = 0;
    for (int i = 0; i < 40; i++)
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
    return length;
  }
}


/****************************************************************************/
/*                             extractPacket()                              */
/*         Issues a new data packet out of a given character string         */
/*            Existing values of parameters will be overridden              */
/*         Pointer passed can be freed once the assignment is done          */
/*          If the packet is formatted successfully, 1 is returned          */
/****************************************************************************/

int ControlPacket::extractPacket(char *final_packet)
{
  if (!final_packet)
    return -1;
  char *temp = (char *)malloc(4*sizeof(char));
  int j = 0;
  int copy = 0;
  int layer = 0;
  while (j < 40)
  {
    for (int i = 0; i < 4; i++)
      *(temp + i) = *(final_packet + i + j);
    memcpy(layers + (9-layer), temp, 4);
    layer++;
    j+=4;
  }
  *m_type = (layers[0] & 0x7FFF0000) >> 16;
  *m_extendedtype = layers[0] & 0x0000FFFF;
  *m_subsequence = layers[1] & 0x7FFFFFFF;
  *m_timestamp = layers[2];
  *m_socketID = layers[3];
  switch (*m_type)
  {
    case 2:   //0010 - Acknowledgement (ACK)
              *m_packetSeq = layers[4];
              *m_RTT = layers[5];
              *m_RTTVar = layers[6];
              *m_availBuffer = layers[7];
              *m_packRecvRate = layers[8];
              *m_linkCap = layers[9];
              break;

    case 6:   //0110 - Acknowledgement of Acknowledgement (ACK-2)
              // ACK packet seq. no.
              break;

    case 3:   //0011 - Loss Report (NAK)
              *m_LossInfo = layers[4];
              break;

    case 4:   //0100 - Congestion Warning
              break;

    case 1:   //0001 - Keep-alive
              break;

    case 0:   //0000 - Handshake
              *m_Version = layers[4];
              *m_Type = layers[5];
              *m_ISN = layers[6];
              *m_MSS = layers[7];
              *m_FlightFlagSize = layers[8];
              *m_ReqType = layers[9];
              break;

    case 5:   //0101 - Shutdown
              break;

    case 7:   //0111 - Message Drop Request
              *m_firstMessage = layers[4];
              *m_lastMessage = layers[5];
              break;

    case 8:   //1000 - Error Signal from the Peer Side
              break;

    default:  break;
  }
  return 1;
}

/****************************************************************************/
/*                              getPacketType()                             */
/*                   Returns the type of control packet                     */
/****************************************************************************/

ControlPacketType ControlPacket::getPacketType()
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
    *m_type = *type;
    return 1;
  }
  else
    return -1;
}

/****************************************************************************/
/*                                setSocketID()                             */
/*        Stores the ID that should be assigned to the said packet          */
/*         If the parameter sequence is NULL, then -1 is returned           */
/*         Pointer passed can be freed once the assignment is done          */
/*               If the ID is set successfully, 1 is returned               */
/****************************************************************************/

int ControlPacket::setSocketID(uint32_t *socketID)
{
  if (socketID)
  {
    *m_socketID = *socketID;
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

int ControlPacket::setControlInfo(ControlPacketType eType, uint32_t *controlinfo)
{
  if (eType == HANDSHAKE && controlinfo)
  {
    *m_Version = *controlinfo++;
    *m_Type = *controlinfo++;
    *m_ISN = *controlinfo++;
    *m_MSS = *controlinfo++;
    *m_FlightFlagSize = *controlinfo++;
    *m_ReqType = *controlinfo++;
    return 1;
  }
  else if (eType == ACK && controlinfo)
  {
    *m_packetSeq = *controlinfo++;
    *m_RTT = *controlinfo++;
    *m_RTTVar = *controlinfo++;
    *m_availBuffer = *controlinfo++;
    *m_packRecvRate = *controlinfo++;
    *m_linkCap = *controlinfo++;
    return 1;
  }
  else if (eType == NAK && controlinfo)
  {
    *m_LossInfo = *controlinfo;
    return 1;
  }
  else if (eType == DropRequest && controlinfo)
  {
    *m_firstMessage = *controlinfo++;
    *m_lastMessage = *controlinfo++;
    return 1;
  }
  else
    return -1;
}

uint32_t ControlPacket::getType()
{
  if (m_type)
    return *m_type;
  else
    return -1;
}

uint32_t ControlPacket::getExtendedType()
{
  if (m_extendedtype)
    return *m_extendedtype;
  else
    return -1;
}

uint32_t ControlPacket::getSubsequence()
{
  if (m_subsequence)
    return *m_subsequence;
  else
    return -1;
}

uint32_t ControlPacket::getTimestamp()
{
  if (m_timestamp)
    return *m_timestamp;
  else
    return -1;
}

uint32_t ControlPacket::getControlInfo(uint32_t *information, int &size)
{
  if (getPacketType() == HANDSHAKE && information)
  {
    *information++ = *m_Version;
    *information++ = *m_Type;
    *information++ = *m_ISN;
    *information++ = *m_MSS;
    *information++ = *m_FlightFlagSize;
    *information++ = *m_ReqType;
    size = 6;
    return 1;
  }
  else if (getPacketType() == ACK && information)
  {
    *information++ = *m_packetSeq;
    *information++ = *m_RTT;
    *information++ = *m_RTTVar;
    *information++ = *m_availBuffer;
    *information++ = *m_packRecvRate;
    *information++ = *m_linkCap;
    size = 6;
    return 1;
  }
  else if (getPacketType() == NAK && information)
  {
    *information++ = *m_LossInfo;
    size = 1;
    return 1;
  }
  else if (getPacketType() == DropRequest && information)
  {
    *information++ = *m_firstMessage;
    *information++ = *m_lastMessage;
    size = 2;
    return 1;
  }
  else
    return -1;
}

uint32_t ControlPacket::getSocketID()
{
  if (m_socketID)
    return *m_socketID;
  else
    return -1;
}
