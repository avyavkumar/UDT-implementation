#include "core.h"

uint64_t UDTCore::current_socket = 0;

std::vector < rec_buffer* > UDTCore::m_recPackets;

std::vector < std::pair <uint64_t, uint32_t> > UDTCore::_successConnect;
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::_currFlowWindowSize;
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::_currPacketSize;

std::vector < std::pair <uint64_t, DataPacket> > UDTCore::LossList;                       // packets lost per socket

std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_subsequence;                    // subsequence of the last packet sent/recv
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_timestamp;                      // latest timestamp

std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_LRSN;                           // ACK - received packets
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_RTT;                            // ACK - RTT
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_RTTVar;                         // ACK - RTTVar
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_availBuffer;                    // ACK - available buffer
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_packRecvRate;                   // ACK - packet receiving rate
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_linkCap;                        // ACK - link capacity
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_Version;                        // HS - UDT version
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_Type;                           // HS - UDT socket type
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_ISN;                            // HS - random initial sequence number
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_MSS;                            // HS - maximum segment size
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_FlightFlagSize;                 // HS - flow control window size
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_ReqType;                        // HS - connection request type:
                                                                                          //  1: regular connection request,
                                                                                          //  0: rendezvous connection request,
                                                                                          //  -1/-2: response
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_LossInfo;                       // NAK - loss list
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_firstMessage;                   // Message Drop - First Message
std::vector < std::pair <uint64_t, uint32_t> > UDTCore::m_lastMessage;                    // Message Drop - Last Message

std::vector < std::pair <uint64_t, sockaddr_in> > UDTCore::m_activeConn;                  // list of active connections

struct CompareFirstSock
{
  CompareFirstSock(uint64_t val) : val_(val) {}
  bool operator()(const std::pair<uint64_t,sockaddr_in> &elem) const {
    return val_ == elem.first;
  }
  private:
    uint64_t val_;
};

struct CompareFirstInt
{
  CompareFirstInt(uint64_t val) : val_(val) {}
  bool operator()(const std::pair<uint64_t,uint32_t> &elem) const {
    return val_ == elem.first;
  }
  private:
    uint64_t val_;
};

struct CompareFirstData
{
  CompareFirstData(uint64_t val) : val_(val) {}
  bool operator()(const std::pair<uint64_t,DataPacket> &elem) const {
    return val_ == elem.first;
  }
  private:
    uint64_t val_;
};

uint64_t UDTCore::hash(uint64_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}

/****************************************************************************/
/*                                  open()                                  */
/*            Returns a pointer to UDTSocket if sucessful binding           */
/*                              NULL otherwise                              */
/****************************************************************************/

UDTSocket* UDTCore::open(int conn_type, int port)
{
  UDTSocket *socket = new UDTSocket();
  socket->setIPVersion(IPv4);
  int _socket = socket->newSocket(conn_type, port);

  if (_socket == -1)
  {
    std::cerr << "UNAVAILABLE MEMORY FOR SOCKET" << std::endl;
    free(socket);
    return NULL;
  }

  int _bind = socket->bindSocket(port);
  if (_bind == -1)
  {
    std::cerr << "UNAVAILABLE MEMORY FOR SOCKET" << std::endl;
    free(socket);
    return NULL;
  }
  return socket;
}

/****************************************************************************/
/*                                connect()                                 */
/*             Connects to a peer given by the input parameter              */
/*   Basically used by client in a client-server role to connect to server  */
/****************************************************************************/

void UDTCore::connect(UDTSocket *socket, const struct sockaddr_in *peer)
{
  if (!socket || !peer)
  {
    std::cerr << "SOCKET/PEER ARE UNALLOC" << std::endl;
    return;
  }
  // intialise HANDSHAKEing information

  UDTCore::current_socket++;
  ControlPacket *packet = new ControlPacket();
  ControlPacket *rec_packet = new ControlPacket();
  uint32_t *type = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *extendedtype = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *subsequence = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *timestamp = (uint32_t *)malloc(sizeof(uint32_t));
  uint64_t *socketID = (uint64_t *)malloc(sizeof(uint64_t));
  uint32_t *controlInfo = (uint32_t *)malloc(6*sizeof(uint32_t));
  *type = HANDSHAKE;
  *extendedtype = 0;
  *subsequence = 0x4324; // initial random subsequence
  *timestamp = std::clock(); // timer mechanism
  *socketID = socket->getSocketID();

  controlInfo[0] = 4;                                             // HS - UDT version
  controlInfo[1] = socket->getFamily();                           // HS - UDT Socket type - 0 - and 1-
  controlInfo[2] = *subsequence;                                  // HS - random initial seq #
  controlInfo[3] = MAXSIZE;                                       // HS - maximum segment size
  controlInfo[4] = 500;                                           // HS - flow control window size
  controlInfo[5] = 1;                                             // HS - connection request type

  int _temp;
  _temp = packet->setType(type);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-TYPE" << std::endl;
    return;
  }
  _temp = packet->setExtendedType(extendedtype);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-EXTENDEDTYPE" << std::endl;
    return;
  }
  _temp = packet->setSubsequence(subsequence);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-SUBSEQUENCE" << std::endl;
    return;
  }
  _temp = packet->setTimestamp(timestamp);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-TIMESTAMP" << std::endl;
    return;
  }
  _temp = packet->setControlInfo(HANDSHAKE, controlInfo);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-CONTROLINFO" << std::endl;
    return;
  }
  _temp = packet->setSocketID(socketID);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-SOCKETID" << std::endl;
    return;
  }

  // extract the packet into a (char*) buffer and send it

  char *s_packet = (char *)malloc(40*sizeof(char));           // buffer for sending the packet
  char *r_packet = (char *)malloc(40*sizeof(char));           // buffer for receiving the packet
  int _bytes = packet->makeControlPacket(s_packet);
  int _step = 0;
  uint32_t can_connect = 0;
  do
  {
    // send the HANDSHAKEing packet
    int _sent = socket->SendPacket(*peer, s_packet, 40);
    if (_sent != 40)
    {
      std::cerr << "ERR-CORRUPT BYTES" << std::endl;
      return;
    }
    // Wait for the return - timer version
    struct sockaddr_in *client = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    int _recv = socket->ReceivePacket(r_packet, client);
    // 40 bytes for control packet; acts as a check
    if (_recv == 40)
    {
      if (getFlag(r_packet, 40) == CONTROL)
      {
        rec_packet->extractPacket(r_packet);
        // check that the packet type is handshake after establishing that it is a control packet
        if(rec_packet->getPacketType() == HANDSHAKE)
        {
          // get the socketID of the packet; it must match with the socketID of the packet sent previously
          if (rec_packet->getSocketID() == packet->getSocketID())
          {
            int size = 0;
            uint32_t *control_info = (uint32_t *)malloc(6*sizeof(uint32_t));
            rec_packet->getControlInfo(control_info, size);
            if (control_info[5] == -1 || control_info[5] == -2)
            {
              std::pair < uint32_t, uint32_t > temp;
              // HS - UDT version
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[0]);
              UDTCore::m_Version.push_back(temp);

              // HS - UDT socket type
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[1]);
              UDTCore::m_Type.push_back(temp);

              // HS - random initial sequence number
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[2]);
              UDTCore::m_ISN.push_back(temp);

              // HS - maximum segment size
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[3]);
              UDTCore::m_MSS.push_back(temp);

              // HS - flow control window size
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[4]);
              UDTCore::m_FlightFlagSize.push_back(temp);

              can_connect = 1;
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), can_connect);
              UDTCore::_successConnect.push_back(temp);

              UDTCore::m_activeConn.push_back(std::make_pair(UDTCore::hash(peer->sin_addr.s_addr), *peer));
              break;
            }
          }
        }
      }
    }
    _step++;
  } while (_step < 20);                         // try this 20 times

  if (can_connect == 0)
    std::cerr << "ERROR-CONNECTION" << std::endl;

  // free memory
  free(packet);
  free(rec_packet);
  free(s_packet);
  free(r_packet);
  free(type);
  free(extendedtype);
  free(subsequence);
  free(timestamp);
  free(socketID);
  free(controlInfo);
}

/****************************************************************************/
/*                                 listen()                                 */
/*              Listens to a peer given by the input parameter              */
/*                      Used EXCLUSIVELY by the server                      */
/*    Used only for control packet types; user/server explicitly needs      */
/*                  to call recv() for data packet types                    */
/****************************************************************************/

void UDTCore::listen(UDTSocket *socket, struct sockaddr_in *peer)
{
  char *buffer = (char *)malloc(40*sizeof(char));
  // control packet
  int size = socket->ReceivePacket(buffer,peer);
  if (getFlag(buffer, size) == CONTROL)
  {
    ControlPacket *packet = new ControlPacket();
    int check = packet->extractPacket(buffer);
    if (packet->getType() == HANDSHAKE)
      UDTCore::connect(socket, peer, buffer);
    else if (packet->getPacketType() == ShutDown)
    {
      //remove the peer from the list of active connections
      std::vector< std::pair <uint64_t, sockaddr_in> >::iterator it;
      it = std::find_if(UDTCore::m_activeConn.begin(), UDTCore::m_activeConn.end(), CompareFirstSock(hash(peer->sin_addr.s_addr)));
      if (it != UDTCore::m_activeConn.end())
        UDTCore::m_activeConn.erase(it);
    }
  }
  if (getFlag(buffer, size) == DATA)
  {
    int length = 0;
    char *data;
    int _length = UDTCore::recv(0,socket,*peer,data,buffer,size,length);
    if (length > 0)
    {
      rec_buffer *r_buffer = (rec_buffer *)malloc(sizeof(rec_buffer));
      r_buffer->m_socketID = hash(socket->getSocketID());
      r_buffer->m_length = length;
      r_buffer->m_buffer = data;
      m_recPackets.push_back(r_buffer);
    }
  }
}

/****************************************************************************/
/*                                connect()                                 */
/*             Connects to a peer given by the input parameter              */
/*   Basically used by server in a client-server role to connect to server  */
/****************************************************************************/

// TODO - intial subsequence number as well as timers

void UDTCore::connect(UDTSocket *socket, const struct sockaddr_in *peer, char *r_packet)
{
  if (!peer || !r_packet)
  {
    std::cerr << "SOCKET/PEER ARE UNALLOC" << std::endl;
    return;
  }
  int _step = 0;
  uint32_t can_connect = 0;
  struct sockaddr_in *client = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  ControlPacket *rec_packet = new ControlPacket();
  do
  {
    int _recv = rec_packet->extractPacket(r_packet);
    // 40 bytes for control packet; acts as a check
    if (_recv == 1)
    {
      // get the flag bit of the packet; check if control bit
      if (getFlag(r_packet, 40) == CONTROL)
      {
        // check that the packet type is handshake after establishing that it is a control packet
        if(rec_packet->getPacketType() == HANDSHAKE)
        {
          // get the socketID of the packet; it must match with the socketID of the packet sent previously
          if (rec_packet->getSocketID() == socket->getSocketID())
          {
            // get the control information of the handshaking packet
            uint32_t *control_info = (uint32_t *)malloc(6*sizeof(uint32_t));
            int size = 0;
            rec_packet->getControlInfo(control_info, size);
            // check whether it is a request type HANDSHAKE
            if (control_info[5] == 1)
            {
              std::pair < uint32_t, uint32_t > temp;
              // HS - UDT version
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[0]);
              UDTCore::m_Version.push_back(temp);

              // HS - UDT socket type
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[1]);
              UDTCore::m_Type.push_back(temp);

              // HS - random initial sequence number
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[2]);
              UDTCore::m_ISN.push_back(temp);

              // HS - maximum segment size
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[3]);
              UDTCore::m_MSS.push_back(temp);

              // HS - flow control window size
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), control_info[4]);
              UDTCore::m_FlightFlagSize.push_back(temp);

              can_connect = 1;
              temp = std::make_pair(UDTCore::hash(rec_packet->getSocketID()), can_connect);
              UDTCore::_successConnect.push_back(temp);

              // add it to the log of active connections
              UDTCore::m_activeConn.push_back(std::make_pair(UDTCore::hash(peer->sin_addr.s_addr), *peer));

              //send a response packet
              ControlPacket *res_packet = new ControlPacket();
              uint32_t *type = (uint32_t *)malloc(sizeof(uint32_t));
              uint32_t *extendedtype = (uint32_t *)malloc(sizeof(uint32_t));
              uint32_t *subsequence = (uint32_t *)malloc(sizeof(uint32_t));
              uint32_t *timestamp = (uint32_t *)malloc(sizeof(uint32_t));
              uint64_t *socketID = (uint64_t *)malloc(sizeof(uint64_t));
              uint32_t *controlInfo = (uint32_t *)malloc(6*sizeof(uint32_t));
              *type = HANDSHAKE;
              *extendedtype = 0;
              *subsequence = 0x4324;     // initial random subsequence
              *timestamp = std::clock(); // timer mechanism
              *socketID = socket->getSocketID();

              controlInfo[0] = 4;                                              // HS - UDT version
              controlInfo[1] = socket->getFamily();                            // HS - UDT Socket type - 0 - and 1-
              controlInfo[2] = *subsequence;                                   // HS - random initial seq #
              std::vector< std::pair <uint64_t, uint32_t> >::iterator it;
              it = std::find_if(UDTCore::_currFlowWindowSize.begin(), UDTCore::_currFlowWindowSize.end(), CompareFirstInt(hash(*socketID)));
              if (it != UDTCore::_currFlowWindowSize.end())
                if (control_info[3] > it->second)                           // HS - maximum segment size
                  controlInfo[3] = it->second;
              else
              {
                controlInfo[3] = control_info[3];
                it->second = control_info[3];
              }

              it = std::find_if(UDTCore::_currFlowWindowSize.begin(), UDTCore::_currFlowWindowSize.end(), CompareFirstInt(hash(*socketID)));
              if (it != UDTCore::_currFlowWindowSize.end())
                if (control_info[4] > it->second)                           // HS - flow control window size
                  controlInfo[4] = it->second;
              else
              {
                controlInfo[4] = control_info[4];
                it->second = control_info[3];
              }
              controlInfo[5] = -1;                                             // HS - connection response type

              int _check;
              _check = res_packet->setType(type);
              if (_check == -1)
                std::cerr << "FAILURE-TYPE" << std::endl;
              _check = res_packet->setExtendedType(extendedtype);
              if (_check == -1)
                std::cerr << "FAILURE-EXTENDEDTYPE" << std::endl;
              _check = res_packet->setSubsequence(subsequence);
              if (_check == -1)
                std::cerr << "FAILURE-SUBSEQUENCE" << std::endl;
              _check = res_packet->setTimestamp(timestamp);
              if (_check == -1)
                std::cerr << "FAILURE-TIMESTAMP" << std::endl;
              _check = res_packet->setControlInfo(HANDSHAKE, controlInfo);
              if (_check == -1)
                std::cerr << "FAILURE-CONTROLINFO" << std::endl;
              _check = res_packet->setSocketID(socketID);
              if (_check == -1)
                std::cerr << "FAILURE-SOCKETID" << std::endl;
              free(type);
              free(extendedtype);
              free(subsequence);
              free(timestamp);
              free(socketID);
              free(controlInfo);
              char *s_packet = (char *)malloc(40*sizeof(char));
              int _bytes_made = res_packet->makeControlPacket(s_packet);
              if (_bytes_made == 40)
              {
                int _bytes_sent = socket->SendPacket(*peer, s_packet, _bytes_made);
                if (_bytes_sent != 40)
                  std::cerr << "FAILURE-CORRUPT BYTES" << std::endl;
              }
              else
                std::cerr << "FAILURE-CORRUPT BYTES" << std::endl;
              free(s_packet);
              free(res_packet);
              break;
            }
          }
        }
      }
    }
    _step++;
  } while (_step < 20);                         // try this 20 times

  if (can_connect == 0)
    std::cerr << "ERROR-CONNECTION" << std::endl;

}

/****************************************************************************/
/*                                 close()                                  */
/*                    Closes the connection with a peer                     */
/****************************************************************************/

void UDTCore::close(UDTSocket *socket, const struct sockaddr_in *peer)
{
  if (!socket || !peer)
  {
    std::cerr << "ERR-NULL MEMORY ASSIGNMENT" << std::endl;
    return;
  }
  ControlPacket *packet = new ControlPacket();
  uint32_t *type = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *extendedtype = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *subsequence = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *timestamp = (uint32_t *)malloc(sizeof(uint32_t));
  uint64_t *socketID = (uint64_t *)malloc(sizeof(uint64_t));
  *type = ShutDown;
  *extendedtype = 0;
  *subsequence = 0xFFFFFFFF; // send any subsequence; doesn't matter which it is
  *timestamp = std::clock(); // timer mechanism
  *socketID = socket->getSocketID();

  int _temp;
  _temp = packet->setType(type);
  if (_temp == -1)
  {
    std::cerr << "CLOSE-FAILURE-TYPE" << std::endl;
    return;
  }
  _temp = packet->setExtendedType(extendedtype);
  if (_temp == -1)
  {
    std::cerr << "CLOSE-FAILURE-EXTENDEDTYPE" << std::endl;
    return;
  }
  _temp = packet->setSubsequence(subsequence);
  if (_temp == -1)
  {
    std::cerr << "CLOSE-FAILURE-SUBSEQUENCE" << std::endl;
    return;
  }
  _temp = packet->setTimestamp(timestamp);
  if (_temp == -1)
  {
    std::cerr << "CLOSE-FAILURE-TIMESTAMP" << std::endl;
    return;
  }
  _temp = packet->setSocketID(socketID);
  if (_temp == -1)
  {
    std::cerr << "CLOSE-FAILURE-SOCKETID" << std::endl;
    return;
  }

  // free memory
  free(type);
  free(extendedtype);
  free(subsequence);
  free(timestamp);
  free(socketID);

  char *s_packet = (char *)malloc(40*sizeof(char));           // buffer for sending the packet
  int _bytes = packet->makeControlPacket(s_packet);
  int _step = 0;
  int _sent;
  do
  {
    _sent = socket->SendPacket(*peer, s_packet, _bytes);
    if (_sent != 40)
      std::cerr << "ERROR-CORRUPT BYTES, TRYING AGAIN..." << std::endl;
    _step++;
  } while (_step < 20 && _sent != 40);                         // try this 20 times

  if (_step == 20)
    std::cerr << "ERROR-COULD NOT SHUT CONNECTION" << std::endl;

  free(packet);
  free(s_packet);

  //remove the peer from the list of active connections
  std::vector< std::pair <uint64_t, sockaddr_in> >::iterator it;
  it = std::find_if(UDTCore::m_activeConn.begin(), UDTCore::m_activeConn.end(), CompareFirstSock(hash(peer->sin_addr.s_addr)));
  if (it != UDTCore::m_activeConn.end())
    UDTCore::m_activeConn.erase(it);
}

/****************************************************************************/
/*                                 send()                                   */
/*             Sends data to a destination contained by socket              */
/*  Assumes that the node has established UDT connection with destination   */
/*                Returns 1 if everything has been sent                     */
/****************************************************************************/

int UDTCore::send(UDTSocket *socket, const struct sockaddr_in peer, char* data, int len)
{
  // close the connection if - all packets are sent or packets are sent partially [give an error message]
  // hash the socketID, find if there are existing packets that need to be sent, if so, then send them first
  DataPacket *packet[20];
  for (int i = 0; i < 20; i++)
    packet[i] = new DataPacket();
  uint32_t total_packets = (len>MAXSIZE)? len/MAXSIZE + ((len%MAXSIZE==0)? 0:1) : 1;
  uint32_t curr_seq = 0;
  uint32_t *sequence = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *funcField = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *orderBit = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *message = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *timestamp = (uint32_t *)malloc(sizeof(uint32_t));
  uint64_t *socketID = (uint64_t *)malloc(sizeof(uint64_t));
  uint32_t packets = 0;
  int len_init, final_length;
  if (total_packets == 1)
    len_init = final_length = len;
  else
  {
    len_init = MAXSIZE;
    final_length = len - (total_packets-1)*MAXSIZE;
  }

  // check the loss list for the socket number
  // if there are some packets, send them first
  std::vector< std::pair <uint64_t, DataPacket> >::iterator it;
  it = std::find_if(UDTCore::LossList.begin(), UDTCore::LossList.end(), CompareFirstData(hash(socket->getSocketID())));
  while (it != UDTCore::LossList.end())
  {
    int _size = it->second.getLength();
    char *send_packet = (char *)((16+_size)*sizeof(char));
    it->second.extractPacket(send_packet, 16 + _size);
    if (socket->SendPacket(peer, send_packet, _size + 16) == _size + 16)
      UDTCore::LossList.erase(it);
    // Need to avait ACK!
    it = std::find_if(UDTCore::LossList.begin(), UDTCore::LossList.end(), CompareFirstData(hash(socket->getSocketID())));
  }

  while (packets < total_packets)
  {
    int i = 0;
    for (i = 0; i < 20; i++)
    {
      if (packets < total_packets)
      {
        if (total_packets == 1)
          *funcField = 3;                             // solo packet sent
        else if (packets == 0 && total_packets > 1)
          *funcField = 2;                             // first packet sent
        else if (packets == total_packets - 1 && total_packets > 1)
          *funcField = 1;                             // last packet sent
        else
          *funcField = 0;                             // packet sent

        *orderBit = 1;                                // in order sending
        *sequence = curr_seq;
        *message = 0xFFFFFFFF;                        // unneeded; this is not a message type
        *timestamp = std::clock();
        *socketID = socket->getSocketID();

        packet[i]->setSequence(sequence);
        packet[i]->setMessage(message);
        packet[i]->setTimestamp(timestamp);
        packet[i]->setfuncField(funcField);
        packet[i]->setOrderBit(orderBit);
        packet[i]->setSocketID(socketID);

        // set the payload
        int nBytes = 0;
        if (total_packets == 1)
        {
          packet[i]->setPayload(data, len);
          char *final_packet = (char *)malloc((len+16)*sizeof(char));
          nBytes = packet[i]->makeDataPacket(final_packet);
          if (nBytes != len + 16)
          {
            std::cerr << "ERROR-CORRUPT PACKET WITH SEQUENCE " <<packet[i]->getSequence()<<" AND SOCKETID "<<packet[i]->getSocketID() << std::endl;
            return -1;
          }
          if (socket->SendPacket(peer, final_packet, final_length+16) != nBytes)
          {
            std::cerr << "ERROR-CORRUPT PACKET SENT WITH SEQUENCE " <<packet[i]->getSequence()<<" AND SOCKETID "<<packet[i]->getSocketID() << std::endl;
            return -1;
          }
          packets++;
          curr_seq++;
          break;
        }
        else if (packets == total_packets - 1)
        {
          packet[i]->setPayload(data+(len_init*packets), final_length);
          char *final_packet = (char *)malloc((final_length+16)*sizeof(char));
          nBytes = packet[i]->makeDataPacket(final_packet);
          std::cout << "HERE1" << std::endl;
          std::cout << nBytes << std::endl;
          if (nBytes != final_length + 16)
          {
            std::cerr << "ERROR-CORRUPT PACKET WITH SEQUENCE " <<packet[i]->getSequence()<<" AND SOCKETID "<<packet[i]->getSocketID() << std::endl;
            return -1;
          }
          if (socket->SendPacket(peer, final_packet, final_length+16) != nBytes)
          {
            std::cerr << "ERROR-CORRUPT PACKET SENT WITH SEQUENCE " <<packet[i]->getSequence()<<" AND SOCKETID "<<packet[i]->getSocketID() << std::endl;
            return -1;
          }
          packets++;
          curr_seq++;
        }
        else
        {
          packet[i]->setPayload(data+(len_init*packets), len_init);
          char *final_packet = (char *)malloc((len_init+16)*sizeof(char));
          nBytes = packet[i]->makeDataPacket(final_packet);
          std::cout << "HERE0" << std::endl;
          if (nBytes != len_init + 16)
          {
            std::cerr << "ERROR-CORRUPT PACKET WITH SEQUENCE " <<packet[i]->getSequence()<<" AND SOCKETID "<<packet[i]->getSocketID() << std::endl;
            return -1;
          }
          int s_size = socket->SendPacket(peer, final_packet, len_init+16);
          if (s_size != nBytes)
          {
            std::cerr << "ERROR-CORRUPT PACKET SENT WITH SEQUENCE " <<packet[i]->getSequence()<<" AND SOCKETID "<<packet[i]->getSocketID() << std::endl;
            return -1;
          }
          packets++;
          std::cout << nBytes << std::endl;
          curr_seq++;
        }
      }
    }

    // Now that 20 packets have been sent, we wait for the response (ACK/NACK/no response)
    // In ACK - all the packets have arrived successfully, we prepare to send the next 20 data packets
    // In NACK - we get the sequence that couldn't be sent. We try again to send them, if still unsuccessful,
    // we place them in the loss list and move on after reporting an error
    // No response - we retransmit the packets

    std::clock_t c_start = std::clock();
    char *recv_packet = (char *)malloc(40*sizeof(char));

    int size = 0;
    struct sockaddr_in *client = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    std::cout << "TRYING TO RECEIVE" << std::endl;
    size = socket->ReceivePacket(recv_packet, client);
    std::cout << "RECEIVED WITH SIZE " << size << std::endl;
    ControlPacket *c_packet = new ControlPacket();
    if (size == -1)
    {
      // nothing received, must retransmit the packets
      packets-=i;
      curr_seq-=i;
    }
    else
    {
      // in order, check if the packet is control type; then check if the socketID matches and then check the ACK, NACK signals
      c_packet->extractPacket(recv_packet);
      while (size == 40)
      {
        if (getFlag(recv_packet, 40) == CONTROL)
        {
          // Case A - packet is ACK
          if(c_packet->getPacketType() == ACK)
          {
            // get the socketID of the packet; it must match with the socketID of the packet sent previously
            if (c_packet->getSocketID() == socket->getSocketID())
            {
              // everything matches; we have an ACK signal for the packets sent
              // here the congestion control activity takes place
              break;
            }
          }
          // Case B- NACK
          else if (c_packet->getPacketType() == NAK)
          {
            // get the sequence numbers of the packets UNACKED, send them again
            // if they are still unacked, then add them to loss list and move on
            if (c_packet->getSocketID() == socket->getSocketID())
            {
              uint32_t *_seq = (uint32_t *)malloc(sizeof(uint32_t));
              int size = 0;
              c_packet->getControlInfo(_seq,size);
              if (size == 1)
              {
                // the packet that needs to be sent again is given by (*_seq)*length
                if (len-((*_seq)+1)*MAXSIZE < 0)
                {
                  // this is the last datapacket
                  DataPacket temp_packet;
                  *funcField = 1;                               // packet sent

                  *orderBit = 1;                                // in order sending
                  *sequence = (*_seq);
                  *message = 0xFFFFFFFF;                        // unneeded; this is not a message type
                  *timestamp = std::clock();
                  *socketID = socket->getSocketID();

                  temp_packet.setSequence(sequence);
                  temp_packet.setMessage(message);
                  temp_packet.setTimestamp(timestamp);
                  temp_packet.setfuncField(funcField);
                  temp_packet.setOrderBit(orderBit);
                  temp_packet.setSocketID(socketID);
                  temp_packet.setPayload(data+MAXSIZE*(*_seq), len-(*_seq)*MAXSIZE);

                  // add this to loss list
                  UDTCore::LossList.push_back(std::make_pair(hash(socket->getSocketID()), temp_packet));
                }
                else
                {
                  // this is the last datapacket
                  DataPacket temp_packet;
                  *funcField = 0;                               // packet sent

                  *orderBit = 1;                                // in order sending
                  *sequence = (*_seq);
                  *message = 0xFFFFFFFF;                        // unneeded; this is not a message type
                  *timestamp = std::clock();
                  *socketID = socket->getSocketID();

                  temp_packet.setSequence(sequence);
                  temp_packet.setMessage(message);
                  temp_packet.setTimestamp(timestamp);
                  temp_packet.setfuncField(funcField);
                  temp_packet.setOrderBit(orderBit);
                  temp_packet.setSocketID(socketID);
                  temp_packet.setPayload(data+MAXSIZE*(*_seq), MAXSIZE);

                  // add this to loss list
                  UDTCore::LossList.push_back(std::make_pair(hash(socket->getSocketID()), temp_packet));
                }
              }
              free(_seq);
            }
          }
        }
        size = socket->ReceivePacket(recv_packet, client);              // Try to get all the NACK packets
      }
    }
  }
  for (int i = 0; i < 20; i++)
    free(packet[i]);
  free(sequence);
  free(funcField);
  free(orderBit);
  free(message);
  free(timestamp);
  free(socketID);
  if (packets == total_packets)
    return 1;
}

/****************************************************************************/
/*                                 recv()                                   */
/*            Receive data to a destination contained by socket             */
/*  Assumes that the node has established UDT connection with destination   */
/*             Returns 1 if everything has been recvd in order              */
/****************************************************************************/

int UDTCore::recv(int mode, UDTSocket *socket, const struct sockaddr_in peer, char* data, char* _data, int r_size, int &length)
{
  #ifndef CLIENT_M
  #define CLIENT_M 1
  #endif

  #ifndef SERVER_M
  #define SERVER_M 0
  #endif

  if (mode == CLIENT_M)
    if (!data || !length <= 0)
    {
      std::cerr << "ERROR-INIT BUFFER EMPTY" << std::endl;
      return -1;
    }

  DataPacket *packet[20];
  ControlPacket *c_packet = new ControlPacket();
  uint32_t *type = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *etype = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *subseq = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *timestamp = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *control_info = (uint32_t *)malloc(6*sizeof(uint32_t));
  uint64_t *socketID = (uint64_t *)malloc(sizeof(uint64_t));
  struct sockaddr_in *client = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  char *s_buffer = (char *)malloc(MAXSIZE*sizeof(char));
  uint32_t *last_seq = (uint32_t *)malloc(sizeof(uint32_t));
  char *final_packet = (char *)malloc(40*sizeof(char));
  char *buffer[20];
  for (int i = 0; i < 20; i++)
    packet[i] = new DataPacket();
  for (int i = 0; i < 20; i++)
    buffer[i] = (char *)malloc((MAXSIZE+16)*sizeof(char));
  int size[20];
  for (int i = 0; i < 20; i++)
    size[i] = -1;
  int packets = 0;
  int _first = 0;
  while(1)
  {
    int last = 0;

    if (mode == SERVER_M && _first == 0)
    {
      size[0] = r_size;
      for (int i = 0; i < r_size; i++)
        buffer[0][i] = _data[i];

      if (size[0] != -1 && getFlag(buffer[0],size[0]) == DATA)
      {
        packet[0]->extractPacket(buffer[0], size[0]);                        // get packet<0>
        packets++;
      }

      if (packet[0]->getfuncField() != 1 && packet[0]->getfuncField() != 3)  // indicates - not the last packet/ solo packet
      {
        for (int i = 1; i < 20; i++)
        {
          if (packet[last]->getfuncField() != 1 && packet[last]->getfuncField() != 3)  // indicates - not the last packet/ solo packet
          {
            size[i] = socket->ReceivePacket(buffer[i], client);
            if (size[i] != -1 && getFlag(buffer[i],size[i]) == DATA)
            {
              packet[i]->extractPacket(buffer[i], size[i]);                    // get as many packets as possible (max is 20)
              packets++;
              last = i;
            }
          }
        }
      }
      _first++;
    }

    else if (mode == CLIENT_M || (mode == SERVER_M && _first != 0))
    {
      for (int i = 0; i < 20; i++)
      {
        size[i] = socket->ReceivePacket(buffer[i], client);
        if (size[i] != -1 && getFlag(buffer[i],size[i]) == DATA)
        {
          packet[i]->extractPacket(buffer[i], size[i]);                  // get as many packets as possible (max is 20)
          packets++;
          last = i;
        }
      }
    }

    if (!packets)
      continue;

    // arrange packets by the sequence numbers received
    DataPacket *temp = new DataPacket();
    int flag = 0;
    for (int i = 0; i < 19; i++)
    {
      for (int j = 0; j < (20-i-1); j++)
      {
        if (size[j] != -1)                                      // need to ensure that packets have been received
        {
          int k;
          for (k = j+1; k < 20-i; k++)
          {
            // std::cout << i << " " << j << " " << k << " " << std::endl;
            if (size[k] != -1)
            {
              flag++;
              break;
            }
          }
          if (flag > 0 && k < 20)
          {
            if (packet[j]->getSequence() > packet[k]->getSequence() && k < (20-i))
            {
              temp = packet[j];
              packet[j] = packet[k];
              packet[k] = temp;
              flag = 0;
            }
          }
        }
      }
    }

    if (packets == 20 && packet[19]->getfuncField() == 1)
    {
      // everything okay, send an ACK signal, it is the last packet
      // the congestion control stuff has been skipped for now
      std::vector< std::pair <uint64_t, uint32_t> >::iterator it;
      it = std::find_if(UDTCore::m_LRSN.begin(), UDTCore::m_LRSN.end(), CompareFirstInt(hash(*socketID)));
      if (it->second + 1 == packet[0]->getSequence() && it != UDTCore::m_LRSN.end())
      {
        *type = ACK;
        *etype = 0xFFFFFFFF;
        *subseq = 0xFFFFFFFF;                                    // subsequence is inconsequential
        *timestamp = std::clock();
        *socketID = socket->getSocketID();
        control_info[0] = packet[19]->getSequence();             // last data packet sequence found

        c_packet->setType(type);
        c_packet->setExtendedType(etype);
        c_packet->setSubsequence(subseq);
        c_packet->setControlInfo(ACK, control_info);
        c_packet->setTimestamp(timestamp);
        c_packet->setSocketID(socketID);

        int length = c_packet->makeControlPacket(final_packet);
        int _step = 0;
        while ( (socket->SendPacket(peer, final_packet, length) == -1) && (_step++ < 20));
        if (_step >= 20)
        {
          std::cerr << "ERROR-COULDN'T SEND ACK" << std::endl;
          return -1;
        }
        UDTCore::m_LRSN.erase(it);
        UDTCore::m_LRSN.push_back(std::make_pair(hash(socket->getSocketID()), control_info[0]));
      }
      // we have sent the ACK, now we need to write the information to buffer
      // to do so, we create a binary file, and dump all the contents into that file
      // once the contents have been transferred and read, the file is removed
      std::fstream myfile;
      myfile.open("write-in.bin", std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app);
      for (int i = 0; i < 20; i++)
      {
        if (size[i] != -1)
        {
          int p_size = packet[i]->getPayload(s_buffer, MAXSIZE);
          myfile << s_buffer;
        }
      }
      myfile.close();
      break;
    }

    else if (packets == 20 && packet[19]->getfuncField() != 1)
    {
      // send an ACK signal, it is not the last packet
      // the congestion control stuff has been skipped for now
      std::vector< std::pair <uint64_t, uint32_t> >::iterator it;
      it = std::find_if(UDTCore::m_LRSN.begin(), UDTCore::m_LRSN.end(), CompareFirstInt(hash(*socketID)));
      if (it->second + 1 == packet[0]->getSequence() && it != UDTCore::m_LRSN.end())
      {
        *type = ACK;
        *etype = 0xFFFFFFFF;
        *subseq = 0xFFFFFFFF;                                    // subsequence is inconsequential
        *timestamp = std::clock();
        *socketID = socket->getSocketID();
        control_info[0] = packet[19]->getSequence();             // last data packet sequence found

        c_packet->setType(type);
        c_packet->setExtendedType(etype);
        c_packet->setSubsequence(subseq);
        c_packet->setControlInfo(ACK, control_info);
        c_packet->setTimestamp(timestamp);
        c_packet->setSocketID(socketID);

        int length = c_packet->makeControlPacket(final_packet);
        int _step = 0;
        while ( (socket->SendPacket(peer, final_packet, length) == -1) && (_step++ < 20));
        if (_step >= 20)
        {
          std::cerr << "ERROR-COULDN'T SEND ACK" << std::endl;
          return -1;
        }
        UDTCore::m_LRSN.erase(it);
        UDTCore::m_LRSN.push_back(std::make_pair(hash(socket->getSocketID()), control_info[0]));
      }
      // packets received are in order
      // to do so, we create a binary file, and dump all the contents into that file
      // once the contents have been transferred and read, the file is removed
      std::fstream myfile;
      myfile.open("write-in.bin", std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app);
      for (int i = 0; i < 20; i++)
      {
        if (size[i] != -1)
        {
          int p_size = packet[i]->getPayload(s_buffer, MAXSIZE);
          myfile << s_buffer;
        }
      }
      myfile.close();
    }

    else if (packets < 20 && packets > 0)
    {
      if (packet[last]->getfuncField() == 1 || packet[last]->getfuncField() == 3)
      {
        std::vector< std::pair <uint64_t, uint32_t> >::iterator it;
        it = std::find_if(UDTCore::m_LRSN.begin(), UDTCore::m_LRSN.end(), CompareFirstInt(hash(*socketID)));
        // Record exists of the socket
        if (it != UDTCore::m_LRSN.end())
        {
          *last_seq = it->second;
          if (*last_seq + packets == packet[last]->getSequence())
          {
            // this is the last packet and we have gotten all the packets correctly
            // send the ACK

            *type = ACK;
            *etype = 0xFFFFFFFF;
            *subseq = 0xFFFFFFFF;                                    // subsequence is inconsequential
            *timestamp = std::clock();
            *socketID = socket->getSocketID();
            control_info[0] = packet[last]->getSequence();             // last data packet sequence found

            c_packet->setType(type);
            c_packet->setExtendedType(etype);
            c_packet->setSubsequence(subseq);
            c_packet->setControlInfo(ACK, control_info);
            c_packet->setTimestamp(timestamp);
            c_packet->setSocketID(socketID);
            int length = c_packet->makeControlPacket(final_packet);
            int _step = 0;
            while ( (socket->SendPacket(peer, final_packet, length) == -1) && (_step++ < 20));
            if (_step >= 20)
            {
              std::cerr << "ERROR-COULDN'T SEND ACK" << std::endl;
              return -1;
            }
            UDTCore::m_LRSN.erase(it);
            UDTCore::m_LRSN.push_back(std::make_pair(hash(socket->getSocketID()), control_info[0]));
            // we have sent the ACK, now we need to write the information to buffer
            // to do so, we create a binary file, and dump all the contents into that file
            // once the contents have been transferred and read, the file is removed
            std::fstream myfile;
            myfile.open("write-in.bin", std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app);
            for (int i = 0; i < 20; i++)
            {
              if (size[i] != -1)
              {
                int p_size = packet[i]->getPayload(s_buffer, MAXSIZE);
                myfile << s_buffer;
              }
            }
            myfile.close();
          }
          else
          {
            std::cerr << "ERROR-MISSING PACKETS; ABORTING" << std::endl;
            return -1;
          }
        }
        // Record doesn't exist within the server
        else
        {
          *type = ACK;
          *etype = 0xFFFFFFFF;
          *subseq = 0xFFFFFFFF;                                      // subsequence is inconsequential
          *timestamp = std::clock();
          *socketID = socket->getSocketID();
          control_info[0] = packet[last]->getSequence();             // last data packet sequence found
          for (int i = 1; i < 6; i++)
            control_info[i] = 0;

          ControlPacket n_packet;
          n_packet.setType(type);
          n_packet.setExtendedType(etype);
          n_packet.setSubsequence(subseq);
          n_packet.setControlInfo(ACK, control_info);
          n_packet.setTimestamp(timestamp);
          n_packet.setSocketID(socketID);
          std::cout << "here #1" << std::endl;
          std::cout << n_packet.getType() << std::endl;
          std::cout << n_packet.getExtendedType() << std::endl;
          std::cout << n_packet.getTimestamp() << std::endl;
          std::cout << n_packet.getSubsequence() << std::endl;
          std::cout << n_packet.getSocketID() << std::endl;
          int wer;
          n_packet.getControlInfo(control_info, wer);
          for (int i = 0; i < wer; i++)
            std::cout << control_info[i] << std::endl;
          int p_length = n_packet.makeControlPacket(final_packet);
          std::cout << "here #2" << std::endl;

          int _step = 0;
          while (_step < 20)
          {
            int p_sent = socket->SendPacket(peer, final_packet, p_length);
            if (p_sent == -1)
              p_sent = socket->SendPacket(peer, final_packet, p_length);
          }
          if (_step >= 20)
          {
            std::cerr << "ERROR-COULDN'T SEND ACK" << std::endl;
            return -1;
          }
          UDTCore::m_LRSN.push_back(std::make_pair(hash(socket->getSocketID()), control_info[0]));
          // we have sent the ACK, now we need to write the information to buffer
          // to do so, we create a binary file, and dump all the contents into that file
          // once the contents have been transferred and read, the file is removed
          std::fstream myfile;
          myfile.open("write-in.bin", std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app);
          for (int i = 0; i < 20; i++)
          {
            if (size[i] != -1)
            {
              int p_size = packet[i]->getPayload(s_buffer, MAXSIZE);
              myfile << s_buffer;
            }
          }
          myfile.close();
        }
        break;
      }
      else
      {
        std::vector< std::pair <uint64_t, uint32_t> >::iterator it;
        it = std::find_if(UDTCore::m_LRSN.begin(), UDTCore::m_LRSN.end(), CompareFirstInt(hash(*socketID)));
        if (it != UDTCore::m_LRSN.end())
        {
          *last_seq = it->second;
          // send a NAK till the last packet received
          // check the difference with the LRSN value
          for (int i = 0; i < 20; i++)
          {
            *type = NAK;
            *etype = 0xFFFFFFFF;
            *subseq = 0xFFFFFFFF;                                    // subsequence is inconsequential
            *timestamp = std::clock();
            *socketID = socket->getSocketID();
            if (size[i] == -1)
              control_info[0] = packet[i]->getSequence();            // last data packet sequence found

            c_packet->setType(type);
            c_packet->setExtendedType(etype);
            c_packet->setSubsequence(subseq);
            c_packet->setControlInfo(NAK, control_info);
            c_packet->setTimestamp(timestamp);
            c_packet->setSocketID(socketID);

            int length = c_packet->makeControlPacket(final_packet);
            int _step = 0;
            while ( (socket->SendPacket(peer, final_packet, length) == -1) && (_step++ < 20));
            if (_step >= 20)
            {
              std::cerr << "ERROR-COULDN'T SEND ACK" << std::endl;
              return -1;
            }
          }
        }
      }
    }
  }
  std::fstream myfile;
  myfile.open("write-in.bin", std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app);
  std::streampos _size = myfile.tellg();
  if (_size > length && mode == CLIENT_M)
  {
    std::cerr << "ERROR-BUFFER INSUFFICIENT" << std::endl;
    myfile.close();
  }
  else if (_size < length && mode == CLIENT_M)
  {
    char *x_data = (char *)malloc(_size*sizeof(char));
    myfile.seekg(0, std::ios::beg);
    myfile.read(x_data, _size);
    myfile.close();
  }
  else if (mode == SERVER_M)
  {
    char *x_data = (char *)malloc(_size*sizeof(char));
    myfile.seekg(0, std::ios::beg);
    myfile.read(x_data, _size);
    myfile.close();
    length = _size;
  }
  return (int)_size;
}
