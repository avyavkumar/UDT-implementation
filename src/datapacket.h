#include "packet.h"
#include <cstring>
#include <cstdlib>
#include <bitset>
#include <stdint.h>
#include <iostream>
#include <stdio.h>

#define MAXSIZE 508

class DataPacket
{
private:
  char *m_packet;
  std::bitset <128> packet;

protected:
  uint32_t *m_sequence;                       // sequence number
  uint32_t *m_message;                        // message number
  uint32_t *m_timestamp;                      // timestamp information
  char *m_packetData;                         // payload
  uint32_t *m_funcField;                      // function field
  uint32_t *m_orderBit;                       // order bit
  uint32_t *m_length;                         // length of the payload
  uint64_t *m_socketID;                       // socketID
  uint32_t layers[4];

public:
  DataPacket();
  ~DataPacket();

  int getLength();
  int makePacket(char *final_packet);
  int setPayload(char *pData, int length);
  int setSequence(uint32_t *sequence);
  int setMessage(uint32_t *message);
  int setTimestamp(uint32_t *timestamp);
  int setfuncField(uint32_t *funcField);
  int setOrderBit(uint32_t *orderBit);
  int setSocketID(uint64_t *socketID);
  int extractPacket(char *final_packet, int length);
  uint32_t getSequence();
  uint32_t getfuncField();
  uint32_t getOrderBit();
  uint32_t getTimestamp();
  uint32_t getMessage();
  uint64_t getSocketID();
  uint32_t getPayload(char *buffer, int length);
};
