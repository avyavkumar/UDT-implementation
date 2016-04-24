#include "socket.h"
#include "controlpacket.h"
#include "datapacket.h"
#include "packet.h"
#include <utility>
#include <chrono>
#include <ctime>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>

typedef struct _rec_buffer
{
  uint64_t m_socketID;
  uint32_t m_length;
  char *m_buffer;
} rec_buffer;

class UDTCore
{
public:
  static uint64_t current_socket;
  static std::vector < std::pair <uint64_t, uint32_t> > _successConnect;
  static std::vector < std::pair <uint64_t, uint32_t> > _currFlowWindowSize;
  static std::vector < std::pair <uint64_t, uint32_t> > _currPacketSize;

  static std::vector < std::pair <uint64_t, DataPacket> > LossList;                       // packets lost per socket

  static std::vector < std::pair <uint64_t, uint32_t> > m_subsequence;                    // subsequence of the last packet sent/recv
  static std::vector < std::pair <uint64_t, uint32_t> > m_timestamp;                      // latest timestamp

  static std::vector <rec_buffer *> m_recPackets;                                         // store buffer of received packets by server

  static std::vector < std::pair <uint64_t, uint32_t> > m_LRSN;                           // ACK - received packets
  static std::vector < std::pair <uint64_t, uint32_t> > m_RTT;                            // ACK - RTT
  static std::vector < std::pair <uint64_t, uint32_t> > m_RTTVar;                         // ACK - RTTVar
  static std::vector < std::pair <uint64_t, uint32_t> > m_availBuffer;                    // ACK - available buffer
  static std::vector < std::pair <uint64_t, uint32_t> > m_packRecvRate;                   // ACK - packet receiving rate
  static std::vector < std::pair <uint64_t, uint32_t> > m_linkCap;                        // ACK - link capacity
  static std::vector < std::pair <uint64_t, uint32_t> > m_Version;                        // HS - UDT version
  static std::vector < std::pair <uint64_t, uint32_t> > m_Type;                           // HS - UDT socket type
  static std::vector < std::pair <uint64_t, uint32_t> > m_ISN;                            // HS - random initial sequence number
  static std::vector < std::pair <uint64_t, uint32_t> > m_MSS;                            // HS - maximum segment size
  static std::vector < std::pair <uint64_t, uint32_t> > m_FlightFlagSize;                 // HS - flow control window size
  static std::vector < std::pair <uint64_t, uint32_t> > m_ReqType;                        // HS - connection request type:
                                                                                          //  1: regular connection request,
                                                                                          //  0: rendezvous connection request,
                                                                                          //  -1/-2: response
  static std::vector < std::pair <uint64_t, uint32_t> > m_LossInfo;                       // NAK - loss list
  static std::vector < std::pair <uint64_t, uint32_t> > m_firstMessage;                   // Message Drop - First Message
  static std::vector < std::pair <uint64_t, uint32_t> > m_lastMessage;                    // Message Drop - Last Message

  static std::vector < std::pair <uint64_t, sockaddr_in> > m_activeConn;                  // list of active connections
  static uint64_t hash(uint64_t x);

  UDTCore();
  static UDTSocket* open(int conn_type, int port);
  static void connect(UDTSocket *socket, const struct sockaddr_in *peer);
  static void connect(UDTSocket *socket, const struct sockaddr_in *peer, char *r_packet);
  static int send(UDTSocket *socket, const struct sockaddr_in peer, char* data, int len);
  static int recv(int mode, UDTSocket *socket, const struct sockaddr_in peer, char* data, char* _data, int r_size, int &length);
  static void close(UDTSocket *socket, const struct sockaddr_in *peer);
  static void listen(UDTSocket *socket, struct sockaddr_in *peer);
  // TODO - implement a method for rendezvous connection
};
