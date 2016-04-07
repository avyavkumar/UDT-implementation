#include "packet.h"

#define MAXSIZE 128

class DataPacket
{
private:
  void *m_packetData;
  std::bitset <96> packet;

protected:
  uint32_t *m_sequence;                       // sequence number
  uint32_t *m_message;                        // message number
  uint32_t *m_timestamp;                      // timestamp information
  uint32_t *m_funcField;                      // function field
  uint32_t *m_orderBit;                       // order bit
  uint32_t layers[3];

public:
  DataPacket();
  ~DataPacket();

  int getLength() const;
  int setLength(int length);
  void makePacket(PacketType eType, uint32_t* ePayload, int size = 0);
  int setPayload(void *pData);
  PacketType getFlag() const;
  int setSequence(uint32_t *sequence);
  int setMessage(uint32_t *message);
  int setTimestamp(uint32_t *timestamp);
  int setfuncField(uint32_t *funcField);
  int setOrderBit(uint32_t *orderBit);
};
