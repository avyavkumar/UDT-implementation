#include "packet.h"
#include <cstdlib.h>
#include <cstdio.h>

class ControlPacket
{

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

  void makePacket(PacketType eType, uint32_t* ePayload, int size = 0);
  PacketType getFlag() const;
  ControlPacketType getType();
  int setType(uint32_t *sequence);
  int setExtendedType(uint32_t *message);
  int setSubsequence(uint32_t *funcField);
  int setTimestamp(uint32_t *timestamp);
  int setControlInfo(uint32_t *orderBit);
};