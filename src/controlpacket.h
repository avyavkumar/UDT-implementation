#include "packet.h"
#include <cstring>
#include <cstdlib>
#include <bitset>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>

#define MAXSIZE 508

class ControlPacket
{
private:
  std::bitset <320> packet;
  char *m_packet;
  friend class HandShakePacket;

protected:
  uint32_t *m_type;                           // type
  uint32_t *m_extendedtype;                   // extended type
  uint32_t *m_subsequence;                    // sub-sequence number
  uint32_t *m_timestamp;                      // timestamp information
  uint64_t *m_socketID;                       // socket ID
  uint32_t *m_packetSeq;                      // ACK - received packets
  uint32_t *m_RTT;                            // ACK - RTT
  uint32_t *m_RTTVar;                         // ACK - RTTVar
  uint32_t *m_availBuffer;                    // ACK - available buffer
  uint32_t *m_packRecvRate;                   // ACK - packet receiving rate
  uint32_t *m_linkCap;                        // ACK - link capacity
  uint32_t *m_Version;                        // HS - UDT version
  uint32_t *m_Type;                           // HS - UDT socket type
  uint32_t *m_ISN;                            // HS - random initial sequence number
  uint32_t *m_MSS;                            // HS - maximum segment size
  uint32_t *m_FlightFlagSize;                 // HS - flow control window size
  uint32_t *m_ReqType;                        // HS - connection request type:
                                              //  1: regular connection request,
                                              //  0: rendezvous connection request,
                                              //  -1/-2: response
  uint32_t *m_LossInfo;                       // NAK - loss information
  uint32_t *m_firstMessage;                   // Message Drop - First Message
  uint32_t *m_lastMessage;                    // Message Drop - Last Message

  uint32_t layers[10];

public:
  ControlPacket();
  ~ControlPacket();

  int makePacket(char *final_packet);
  int extractPacket(char *final_packet);
  ControlPacketType getPacketType();
  int setType(uint32_t *type);
  int setExtendedType(uint32_t *extendedtype);
  int setSubsequence(uint32_t *subsequence);
  int setTimestamp(uint32_t *timestamp);
  int setControlInfo(ControlPacketType eType, uint32_t *controlinfo);
  int setSocketID(uint64_t *socketID);
  uint32_t getType();
  uint32_t getExtendedType();
  uint32_t getSubsequence();
  uint32_t getTimestamp();
  uint32_t getControlInfo(uint32_t *information, int &size);
  uint64_t getSocketID();
};
