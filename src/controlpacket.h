#include "packet.h"

#define MAXSIZE 508

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
  int extractPacket(char *final_packet);
  PacketType getFlag();
  ControlPacketType getPacketType();
  int setType(uint32_t *type);
  int setExtendedType(uint32_t *message);
  int setSubsequence(uint32_t *funcField);
  int setTimestamp(uint32_t *timestamp);
  int setControlInfo(uint32_t *controlinfo);
  uint32_t getType();
  uint32_t getExtendedType();
  uint32_t getSubsequence();
  uint32_t getTimestamp();
  uint32_t getControlInfo();

};
