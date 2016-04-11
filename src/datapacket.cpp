#include "datapacket.h"
#include <cstring>
#include <cstdlib>
#include <bitset>
#include <stdint.h>
#include <iostream>

// DATA PACKET FORMAT IS AS FOLLOWS

/****************************************************************************/
/*      0                   1                   2                   3       */
/*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1       */
/*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      */
/*   |0|                        Sequence Number                      |      */
/*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      */
/*   |ff |o|                     Message Number                      |      */
/*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      */
/*   |                          Time Stamp                           |      */
/*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      */
/*                                                                          */
/****************************************************************************/


/****************************************************************************/
/*                           DataPacket()                                   */
/*                           Constructor                                    */
/****************************************************************************/

DataPacket::DataPacket():
m_sequence(NULL),
m_message(NULL),
m_timestamp(NULL),
m_packetData(NULL),
m_funcField(NULL),
m_orderBit(NULL),
m_packet(NULL)
{
  for (int i = 0; i < 3; i++)
    layers[i] = 0;
}

/****************************************************************************/
/*                           DataPacket()                                   */
/*                            Destructor                                    */
/****************************************************************************/

DataPacket::~DataPacket() {}

/****************************************************************************/
/*                           getLength()                                    */
/*         Get the payload length of the data packet in bytes.              */
/*     If the payload has not been specified, then -1 is returned           */
/****************************************************************************/

int DataPacket::getLength()
{
  if (m_packetData)
    return sizeof(m_packetData);
  else
    return -1;
}

/****************************************************************************/
/*                              makePacket()                                */
/*                        Construct the data packet                         */
/*           The final packet is returned along with the padded bits        */
/*      All the fields must be defined, otherwise NULL will be returned     */
/****************************************************************************/

int DataPacket::makePacket(char *final_packet)
{
  if (!m_sequence || !m_message || !m_timestamp || !m_packetData || !m_orderBit || !m_funcField)
    return -1;

  // set the type as data; make the first bit of the packet 0
  // make the next 31 bits the sequence number
  // set the next two bits as function field
  // set the ordering priority as the next bit
  // set the next 29 bits as message sequence
  // issue a time stamp for the next 32 bits
  // finally, copy the payload and return a pointer to the packet

  // make a temporary string for the second row of the data packet
  uint32_t *temp_1 = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *temp_2 = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *temp_3 = (uint32_t *)malloc(sizeof(uint32_t));
  *temp_1 = (*m_funcField & 0x0003) << 30;
  *temp_2 = (*m_orderBit & 0x0001) << 29;
  *temp_3 =  0x1FFFFFFF & *m_message;
  *temp_1 = *temp_1 | *temp_2 | *temp_3;

  layers[0] = 0x7FFFFFFF & *m_sequence;
  layers[1] = *temp_1;
  layers[2] = *m_timestamp;

  free(temp_1);
  free(temp_2);
  free(temp_3);

  std::bitset <96> tempo_1 (layers[0]);
  std::bitset <96> tempo_2 (layers[1]);
  std::bitset <96> tempo_3 (layers[2]);
  tempo_1 <<= 64;
  tempo_2 <<= 32;
  int length = 0;
  packet = tempo_1 | tempo_2 | tempo_3;

  for (int i = 0; i < 12; i++)
  {
    std::bitset<8> c;
    int q = i*8;
    for (int j = 0; j < 8; j++)
      c[j] = packet[q++];
    m_packet[length] = char(c.to_ulong());
    length++;
  }

  for (int i = 0; i < sizeof(m_packetData); i++)
    *(m_packet + 12 + i) = *(m_packetData + i);

  for (int i = 0; i < 12+sizeof(m_packetData); i++)
    *(final_packet + i) = *(m_packet + i);

  return length+sizeof(m_packetData);
}

/****************************************************************************/
/*                              setPayload()                                */
/*                         Construct the payload                            */
/*          Pointer passed can be freed once the assignment is done         */
/****************************************************************************/

int DataPacket::setPayload(char *poData)
{
  if (poData)
  {
    int length = sizeof(poData);
    m_packetData = (char *)malloc(length*sizeof(char));
    m_packet = (char *)malloc((12+length)*sizeof(char));
    for (int i = 0; i < length; i++)
      *(m_packetData + i) = *(poData + i);
    return 1;
  }
  else
    return -1;
}

/****************************************************************************/
/*                              getFlag()                                   */
/*                        Returns the type of flag                          */
/****************************************************************************/

PacketType DataPacket::getFlag()
{
  return DATA;
}

/****************************************************************************/
/*                              setSequence()                               */
/*   Stores the sequence number that should be assigned to the said packet  */
/*         If the parameter sequence is NULL, then -1 is returned           */
/*          Pointer passed can be freed once the assignment is done         */
/*          If the sequence is set successfully, 1 is returned              */
/****************************************************************************/

int DataPacket::setSequence(uint32_t *sequence)
{
  if (sequence)
  {
    m_sequence = (uint32_t *)malloc(sizeof(uint32_t));
    *m_sequence = *sequence;
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

int DataPacket::setTimestamp(uint32_t *timestamp)
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
/*                              setfuncField()                              */
/*   Stores the funcField that should be assigned to the said packet        */
/*         If the parameter funcField is NULL, then -1 is returned          */
/*          Pointer passed can be freed once the assignment is done         */
/*          If the funcField is set successfully, 1 is returned             */
/****************************************************************************/

int DataPacket::setfuncField(uint32_t *funcField)
{
  if (funcField)
  {
    m_funcField = (uint32_t *)malloc(sizeof(uint32_t));
    *m_funcField = *funcField;
    return 1;
  }
  else
    return -1;
}

/****************************************************************************/
/*                               setMessage()                               */
/*       Stores the message that should be assigned to the said packet      */
/*           If the parameter message is NULL, then -1 is returned          */
/*          Pointer passed can be freed once the assignment is done         */
/*            If the message is set successfully, 1 is returned             */
/****************************************************************************/

int DataPacket::setMessage(uint32_t *message)
{
  if (message)
  {
    m_message = (uint32_t *)malloc(sizeof(uint32_t));
    *m_message = *message;
    return 1;
  }
  else
    return -1;
}

/****************************************************************************/
/*                              setOrderBit()                               */
/*     Stores the order bit that should be assigned to the said packet      */
/*         If the parameter orderBit is NULL, then -1 is returned           */
/*         Pointer passed can be freed once the assignment is done          */
/*           If the orderBit is set successfully, 1 is returned             */
/****************************************************************************/

int DataPacket::setOrderBit(uint32_t *orderBit)
{
  if (orderBit)
  {
    m_orderBit = (uint32_t *)malloc(sizeof(uint32_t));
    *m_orderBit = *orderBit;
    return 1;
  }
  else
    return -1;
}
