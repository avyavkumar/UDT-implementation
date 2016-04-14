#include "socket.h"
#include "controlpacket.h"
#include "datapacket.h"
#include "handshakepacket.h"
#include "packet.h"

class UDTCore
{
  // Functionality:``
  //    initialize a UDT entity and bind to a local address.
  // Parameters:
  //    None.
  // Returned value:
  //    None.

  int open(UDTSocket *socket);

  // Functionality:
  //    Start listening to any connection request.
  // Parameters:
  //    None.
  // Returned value:
  //    None.

  int listen(UDTSocket *socket);

  // Functionality:
  //    Connect to a UDT entity listening at address "peer".
  // Parameters:
  //    0) [in] peer: The address of the listening UDT entity.
  // Returned value:
  //    None.

  void connect(const sockaddr* peer);

  // Functionality:
  //    Process the response handshake packet.
  // Parameters:
  //    0) [in] pkt: handshake packet.
  // Returned value:
  //    Return 0 if connected, positive value if connection is in progress, otherwise error code.

  int connect(const CPacket& pkt) throw ();

  // Functionality:
  //    Connect to a UDT entity listening at address "peer", which has sent "hs" request.
  // Parameters:
  //    0) [in] peer: The address of the listening UDT entity.
  //    1) [in/out] hs: The handshake information sent by the peer side (in), negotiated value (out).
  // Returned value:
  //    None.

  void connect(const sockaddr* peer, CHandShake* hs);

  // Functionality:
  //    Close the opened UDT entity.
  // Parameters:
  //    None.
  // Returned value:
  //    None.

  void close();

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
};

