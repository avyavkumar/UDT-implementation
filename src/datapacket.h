#include "packet.h"

#define MAXSIZE 128

class DataPacket
{
private:
  char *m_packet;
  std::bitset <96> packet;

protected:
  uint32_t *m_sequence;                       // sequence number
  uint32_t *m_message;                        // message number
  uint32_t *m_timestamp;                      // timestamp information
  char *m_packetData;                         // payload
  uint32_t *m_funcField;                      // function field
  uint32_t *m_orderBit;                       // order bit
  uint32_t layers[3];

public:
  DataPacket();
  ~DataPacket();

  int getLength();
  int makePacket(char *final_packet);
  int setPayload(char *pData);
  PacketType getFlag();
  int setSequence(uint32_t *sequence);
  int setMessage(uint32_t *message);
  int setTimestamp(uint32_t *timestamp);
  int setfuncField(uint32_t *funcField);
  int setOrderBit(uint32_t *orderBit);
};
