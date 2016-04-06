#include "datapacket.h"
#include <cstring>
#include <cstdlib>

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

int DataPacket::getLength() const;
{
  if (m_packetData)
    return sizeof(m_packetData);
  else
    return -1;
}

/****************************************************************************/
/*                           setLength()                                    */
/*         Set the payload length of the data packet in bytes.              */
/*     If the payload has not been specified, then only it is set           */
/*   If the payload exists beforehand, then error code -1 is returned       */
/*            Maximum payload that can be set is 128 bytes                  */
/****************************************************************************/

int DataPacket::setLength(int length)
{
  if (m_packetData)
    return -1;
  else
  {
    if (length < 128)
      m_packetData = malloc(length*sizeof(char));
    else
      m_packetData = malloc(MAXSIZE*sizeof(char));
  }
}

/****************************************************************************/
/*                              makePacket()                                */
/*                        Construct the data packet                         */
/*           The final packet is returned along with the padded bits        */
/*      All the fields must be defined, otherwise NULL will be returned     */
/****************************************************************************/

char* DataPacket::makePacket(char* ePayload, int size = 0)
{
  if (!m_sequence || !m_message || !m_timestamp || !m_packetData || !m_orderBit || !m_funcField)
    return NULL;

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
  *temp_3 =  0x1FFF & *m_message;
  *temp_1 = *temp_1 | *temp_2 | *temp_3;

  layers[0] = 0x7FFF & *m_sequence;
  layers[1] = *temp_1;
  layers[2] = *m_timestamp;

  free(temp_1);
  free(temp_2);
  free(temp_3);

  //TODO - merge the layers into one concise buffer; preferably of type (void*) to manage endianness
}

/****************************************************************************/
/*                              setPayload()                                */
/*                         Construct the payload                            */
/*          Pointer passed can be freed once the assignment is done         */
/****************************************************************************/

int DataPacket::setPayload(void *poData)
{
  *m_packetData = *poData;
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
