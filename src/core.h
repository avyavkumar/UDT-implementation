#include "socket.h"
#include "controlpacket.h"
#include "datapacket.h"
#include "packet.h"
#include <utility>
#include <chrono>
#include <ctime>
#include <vector>
#include <algorithm>

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
  static void connect(UDTSocket *socket, const sockaddr_in *peer);
  static void connect(UDTSocket *socket, const sockaddr_in *peer, ControlPacket *rec_packet);
  // TODO - implement a method for rendezvous connection
  static void close(UDTSocket *socket, const sockaddr_in *peer);
  static int send(UDTSocket *socket, const struct sockaddr_in peer, char* data, int len);
  static int recv(UDTSocket *socket, const struct sockaddr_in peer, char* data, int len);
  // TODO - write a sendmsg method
  // TODO - write a recvmsg method

  /*
  // Functionality:
  //    Start listening to any connection request.
  // Parameters:
  //    None.
  // Returned value:
  //    None.

  void listen();

  // Functionality:
  //    Request UDT to send out a data block "data" with size of "len".
  // Parameters:
  //    0) [in] data: The address of the application data to be sent.
  //    1) [in] len: The size of the data block.
  // Returned value:
  //    Actual size of data sent.

  int send(const char* data, int len);

  // Functionality:
  //    Request UDT to receive data to a memory block "data" with size of "len".
  // Parameters:
  //    0) [out] data: data received.
  //    1) [in] len: The desired size of data to be received.
  // Returned value:
  //    Actual size of data received.

  int recv(char* data, int len);

  // Functionality:
  //    send a message of a memory block "data" with size of "len".
  // Parameters:
  //    0) [out] data: data received.
  //    1) [in] len: The desired size of data to be received.
  //    2) [in] ttl: the time-to-live of the message.
  //    3) [in] inorder: if the message should be delivered in order.
  // Returned value:
  //    Actual size of data sent.

  int sendmsg(const char* data, int len, int ttl, bool inorder);

  // Functionality:
  //    Receive a message to buffer "data".
  // Parameters:
  //    0) [out] data: data received.
  //    1) [in] len: size of the buffer.
  // Returned value:
  //    Actual size of data received.

  int recvmsg(char* data, int len);

  // Functionality:
  //    Request UDT to send out a file described as "fd", starting from "offset", with size of "size".
  // Parameters:
  //    0) [in] ifs: The input file stream.
  //    1) [in, out] offset: From where to read and send data; output is the new offset when the call returns.
  //    2) [in] size: How many data to be sent.
  //    3) [in] block: size of block per read from disk
  // Returned value:
  //    Actual size of data sent.

  int64_t sendfile(std::fstream& ifs, int64_t& offset, int64_t size, int block = 366000);

  // Functionality:
  //    Request UDT to receive data into a file described as "fd", starting from "offset", with expected size of "size".
  // Parameters:
  //    0) [out] ofs: The output file stream.
  //    1) [in, out] offset: From where to write data; output is the new offset when the call returns.
  //    2) [in] size: How many data to be received.
  //    3) [in] block: size of block per write to disk
  // Returned value:
  //    Actual size of data received.

  int64_t recvfile(std::fstream& ofs, int64_t& offset, int64_t size, int block = 7320000);

  // Functionality:
  //    Configure UDT options.
  // Parameters:
  //    0) [in] optName: The enum name of a UDT option.
  //    1) [in] optval: The value to be set.
  //    2) [in] optlen: size of "optval".
  // Returned value:
  //    None.

  void setOpt(UDTOpt optName, const void* optval, int optlen);

  // Functionality:
  //    Read UDT options.
  // Parameters:
  //    0) [in] optName: The enum name of a UDT option.
  //    1) [in] optval: The value to be returned.
  //    2) [out] optlen: size of "optval".
  // Returned value:
  //    None.

  void getOpt(UDTOpt optName, void* optval, int& optlen);

  // Functionality:
  //    read the performance data since last sample() call.
  // Parameters:
  //    0) [in, out] perf: pointer to a CPerfMon structure to record the performance data.
  //    1) [in] clear: flag to decide if the local performance trace should be cleared.
  // Returned value:
  //    None.

  void sample(CPerfMon* perf, bool clear = true);
  */
};
