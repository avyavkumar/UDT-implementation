#include "core.h"

uint64_t UDTCore::current_socket = 0;

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

UDTCore::UDTCore() {}

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

void UDTCore::connect(UDTSocket *socket, const sockaddr_in *peer)
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

  controlInfo[0] = 4;                                              // HS - UDT version
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

  // free memory
  free(type);
  free(extendedtype);
  free(subsequence);
  free(timestamp);
  free(socketID);
  for (int i = 0; i < 6; i++)
    free(controlInfo+i);

  // extract the packet into a (char*) buffer and send it

  char *s_packet = (char *)malloc(40*sizeof(char));           // buffer for sending the packet
  char *r_packet = (char *)malloc(40*sizeof(char));           // buffer for receiving the packet
  int _bytes = packet->extractPacket(s_packet);
  int _step = 0;
  uint32_t can_connect = 0;
  do
  {
    // send the HANDSHAKEing packet
    int _sent = socket->SendPacket(*peer, s_packet, _bytes);
    if (_sent != 40)
    {
      std::cerr << "ERR-CORRUPT BYTES" << std::endl;
      return;
    }
    // intialise the timer
    std::clock_t c_start = std::clock();
    while (std::clock() - c_start < 1000);

    // Wait for the return - timer version
    int _recv = socket->ReceivePacket(r_packet);
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

              m_activeConn.push_back(std::make_pair(UDTCore::hash(peer->sin_addr.s_addr), *peer));
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

  free(packet);
  free(rec_packet);
  free(s_packet);
  free(r_packet);
}

/****************************************************************************/
/*                                connect()                                 */
/*             Connects to a peer given by the input parameter              */
/*   Basically used by server in a client-server role to connect to server  */
/****************************************************************************/

// TODO - intial subsequence number as well as timers

void UDTCore::connect(UDTSocket *socket, const sockaddr_in *peer, ControlPacket *rec_packet)
{
  if (!peer || !rec_packet)
  {
    std::cerr << "SOCKET/PEER ARE UNALLOC" << std::endl;
    return;
  }
  int _step = 0;
  uint32_t can_connect = 0;
  do
  {
    // intialise the timer
    std::clock_t c_start = std::clock();
    while (std::clock() - c_start < 1000);

    char *r_packet = (char *)malloc(40*sizeof(char));
    int _recv = rec_packet->extractPacket(r_packet);
    // 40 bytes for control packet; acts as a check
    if (_recv == 40)
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
              m_activeConn.push_back(std::make_pair(UDTCore::hash(peer->sin_addr.s_addr), *peer));

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
              it = std::find_if(_currPacketSize.begin(), _currPacketSize.end(), CompareFirstInt(hash(*socketID)));
              if (it != _currPacketSize.end())
                if (control_info[3] > it->second)                           // HS - maximum segment size
                  controlInfo[3] = it->second;
              else
              {
                controlInfo[3] = control_info[3];
                it->second = control_info[3];
              }

              it = std::find_if(_currFlowWindowSize.begin(), _currFlowWindowSize.end(), CompareFirstInt(hash(*socketID)));
              if (it != _currFlowWindowSize.end())
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
              for (int i = 0; i < 6; i++)
                free(controlInfo+i);
              char *s_packet = (char *)malloc(40*sizeof(char));
              int _bytes_made = res_packet->makePacket(s_packet);
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

void UDTCore::close(UDTSocket *socket, const sockaddr_in *peer)
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
  int _bytes = packet->extractPacket(s_packet);
  int _step = 0;

  do
  {
    int _sent = socket->SendPacket(*peer, s_packet, _bytes);
    if (_sent != 40)
      std::cerr << "ERROR-CORRUPT BYTES, TRYING AGAIN..." << std::endl;
    _step++;
  } while (_step < 20);                         // try this 20 times

  if (_step == 20)
    std::cerr << "ERROR-COULD NOT SHUT CONNECTION" << std::endl;

  free(packet);
  free(s_packet);

  //remove the peer from the list of active connections
  std::vector< std::pair <uint64_t, sockaddr_in> >::iterator it;
  it = std::find_if(m_activeConn.begin(), m_activeConn.end(), CompareFirstSock(hash(peer->sin_addr.s_addr)));
  if (it != m_activeConn.end())
    m_activeConn.erase(it);
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
  uint32_t *m_sequence = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *m_funcField = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *m_orderBit = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *m_message = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *m_timestamp = (uint32_t *)malloc(sizeof(uint32_t));
  uint64_t *m_socketID = (uint64_t *)malloc(sizeof(uint64_t));
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
  it = std::find_if(LossList.begin(), LossList.end(), CompareFirstData(hash(socket->getSocketID())));
  while (it != LossList.end())
  {
    int _size = it->second.getLength();
    char *send_packet = (char *)((16+_size)*sizeof(char));
    it->second.extractPacket(send_packet, 16 + _size);
    if (socket->SendPacket(peer, send_packet, _size + 16) == _size + 16)
      LossList.erase(it);
    it = std::find_if(LossList.begin(), LossList.end(), CompareFirstData(hash(socket->getSocketID())));
  }

  while (packets < total_packets)
  {
    int i = 0;
    for (i = 0; i < 20; i++)
    {
      if (packets < total_packets)
      {
        if (total_packets == 1)
          *m_funcField = 3;                             // solo packet sent
        else if (!packets && total_packets > 1)
          *m_funcField = 2;                             // first packet sent
        else if (packets == total_packets - 1 && total_packets > 1)
          *m_funcField = 1;                             // last packet sent
        else
          *m_funcField = 0;                             // packet sent

        *m_orderBit = 1;                                // in order sending
        *m_sequence = curr_seq;
        *m_message = 0xFFFFFFFF;                        // unneeded; this is not a message type
        *m_timestamp = std::clock();
        *m_socketID = socket->getSocketID();

        packet[i]->setSequence(m_sequence);
        packet[i]->setMessage(m_message);
        packet[i]->setTimestamp(m_timestamp);
        packet[i]->setfuncField(m_funcField);
        packet[i]->setOrderBit(m_orderBit);
        packet[i]->setSocketID(m_socketID);

        // set the payload
        int nBytes = 0;
        if (total_packets == 1)
        {
          packet[i]->setPayload(data, len);
          char *final_packet = (char *)malloc((len+16)*sizeof(char));
          nBytes = packet[i]->makePacket(final_packet);
          if (nBytes != len + 16)
          {
            std::cerr << "ERROR-CORRUPT PACKET WITH SEQUENCE " <<packet[i]->getSequence()<<" AND SOCKETID "<<packet[i]->getSocketID() << std::endl;
            return -1;
          }
          socket->SendPacket(peer, final_packet, len+16);
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
          nBytes = packet[i]->makePacket(final_packet);
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
          break;
        }
        else
        {
          packet[i]->setPayload(data+(len_init*packets), len_init);
          char *final_packet = (char *)malloc((len_init+16)*sizeof(char));
          nBytes = packet[i]->makePacket(final_packet);
          if (nBytes != len_init + 16)
          {
            std::cerr << "ERROR-CORRUPT PACKET WITH SEQUENCE " <<packet[i]->getSequence()<<" AND SOCKETID "<<packet[i]->getSocketID() << std::endl;
            return -1;
          }
          socket->SendPacket(peer, final_packet, len_init+16);
          if (socket->SendPacket(peer, final_packet, final_length+16) != nBytes)
          {
            std::cerr << "ERROR-CORRUPT PACKET SENT WITH SEQUENCE " <<packet[i]->getSequence()<<" AND SOCKETID "<<packet[i]->getSocketID() << std::endl;
            return -1;
          }
          packets++;
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
    while (std::clock() - c_start < 20000);                            // Wait for some time for a response
    char *recv_packet = (char *)malloc(40*sizeof(char));

    int size = 0;
    size = socket->ReceivePacket(recv_packet);
    ControlPacket *c_packet = new ControlPacket();
    if (c_packet->extractPacket(recv_packet) == -1)
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
                  *m_funcField = 1;                               // packet sent

                  *m_orderBit = 1;                                // in order sending
                  *m_sequence = (*_seq);
                  *m_message = 0xFFFFFFFF;                        // unneeded; this is not a message type
                  *m_timestamp = std::clock();
                  *m_socketID = socket->getSocketID();

                  temp_packet.setSequence(m_sequence);
                  temp_packet.setMessage(m_message);
                  temp_packet.setTimestamp(m_timestamp);
                  temp_packet.setfuncField(m_funcField);
                  temp_packet.setOrderBit(m_orderBit);
                  temp_packet.setSocketID(m_socketID);
                  temp_packet.setPayload(data+MAXSIZE*(*_seq), len-(*_seq)*MAXSIZE);

                  // add this to loss list
                  LossList.push_back(std::make_pair(hash(socket->getSocketID()), temp_packet));
                }
                else
                {
                  // this is the last datapacket
                  DataPacket temp_packet;
                  *m_funcField = 0;                               // packet sent

                  *m_orderBit = 1;                                // in order sending
                  *m_sequence = (*_seq);
                  *m_message = 0xFFFFFFFF;                        // unneeded; this is not a message type
                  *m_timestamp = std::clock();
                  *m_socketID = socket->getSocketID();

                  temp_packet.setSequence(m_sequence);
                  temp_packet.setMessage(m_message);
                  temp_packet.setTimestamp(m_timestamp);
                  temp_packet.setfuncField(m_funcField);
                  temp_packet.setOrderBit(m_orderBit);
                  temp_packet.setSocketID(m_socketID);
                  temp_packet.setPayload(data+MAXSIZE*(*_seq), MAXSIZE);

                  // add this to loss list
                  LossList.push_back(std::make_pair(hash(socket->getSocketID()), temp_packet));
                }
              }
              free(_seq);
            }
          }
        }
        size = socket->ReceivePacket(recv_packet);              // Try to get all the NACK packets
      }
    }
  }
  for (int i = 0; i < 20; i++)
    free(packet[i]);
  free(m_sequence);
  free(m_funcField);
  free(m_orderBit);
  free(m_message);
  free(m_timestamp);
  free(m_socketID);
  if (packets == total_packets)
    return 1;
}

/****************************************************************************/
/*                                 recv()                                   */
/*            Receive data to a destination contained by socket             */
/*  Assumes that the node has established UDT connection with destination   */
/*             Returns 1 if everything has been recvd in order              */
/****************************************************************************/

int UDTCore::recv(UDTSocket *socket, const struct sockaddr_in peer, char* data, int length)
{
  if (!data)
  {
    std::cerr << "ERROR-BUFFER EMPTY" << std::endl;
    return -1;
  }
  DataPacket *packet[20];
  char *buffer[20];
  for (int i = 0; i < 20; i++)
    packet[i] = new DataPacket();
  for (int i = 0; i < 20; i++)
    buffer[i] = (char *)malloc((MAXSIZE+16)*sizeof(char));
  int size[20];
  int packets = 0;
  ControlPacket *c_packet = new ControlPacket();
  uint32_t *type = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *etype = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *subseq = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *timestamp = (uint32_t *)malloc(sizeof(uint32_t));
  uint32_t *control_info = (uint32_t *)malloc(6*sizeof(uint32_t));
  uint64_t *socketID = (uint64_t *)malloc(sizeof(uint64_t));

  while(1)
  {
    std::clock_t c_start = std::clock();
    while (std::clock() - c_start < 20000);                            // Wait for some time for a response
    int last = 0;
    for (int i = 0; i < 20; i++)
    {
      size[i] = socket->ReceivePacket(buffer[i]);
      if (size[i] != -1 && getFlag(buffer[i],size[i]) == DATA)
      {
        packet[i]->extractPacket(buffer[i], size[i]);          // get as many packets as possible (max is 20)
        packets++;
        last = i;
      }
    }

    if (!packets)
      continue;

    // arrange packets by the sequence numbers received
    DataPacket *temp = new DataPacket();
    for (int i = 0; i < 20; i++)
    {
      for (int j = 0; j < (20-i); j++)
      {
        if (size[j] != -1)                                      // need to ensure that packets have been received
        {
          int k;
          for (k = j+1; k < 20-i; k++)
          {
            if (size[k] != -1)
              break;
          }
          if (packet[j]->getSequence() > packet[k]->getSequence() && k < (20-i))
          {
            temp = packet[j];
            packet[j] = packet[k];
            packet[k] = temp;
          }
        }
      }
    }
    free(temp);
    if (packets == 20 && packet[19]->getfuncField() == 1)
    {
      // everything okay, send an ACK signal, it is the last packet
      // the congestion control stuff has been skipped for now
      std::vector< std::pair <uint64_t, uint32_t> >::iterator it;
      it = std::find_if(m_LRSN.begin(), m_LRSN.end(), CompareFirstInt(hash(*socketID)));
      if (it->second + 1 == packet[0]->getSequence() && it != m_LRSN.end())
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

        char *final_packet = (char *)malloc(40*sizeof(char));
        int length = c_packet->makePacket(final_packet);
        int _step = 0;
        while ( (socket->SendPacket(peer, final_packet, length) == -1) && (_step++ < 20));
        if (_step >= 20)
        {
          std::cerr << "ERROR-COULDN'T SEND ACK" << std::endl;
          return -1;
        }
        m_LRSN.erase(it);
        m_LRSN.push_back(std::make_pair(hash(socket->getSocketID()), control_info[0]));
        free(final_packet);
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
          char *s_buffer = (char *)malloc(MAXSIZE*sizeof(char));
          int p_size = packet[i]->getPayload(s_buffer, MAXSIZE);
          myfile << s_buffer;
          free(s_buffer);
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
      it = std::find_if(m_LRSN.begin(), m_LRSN.end(), CompareFirstInt(hash(*socketID)));
      if (it->second + 1 == packet[0]->getSequence() && it != m_LRSN.end())
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

        char *final_packet = (char *)malloc(40*sizeof(char));
        int length = c_packet->makePacket(final_packet);
        int _step = 0;
        while ( (socket->SendPacket(peer, final_packet, length) == -1) && (_step++ < 20));
        if (_step >= 20)
        {
          std::cerr << "ERROR-COULDN'T SEND ACK" << std::endl;
          return -1;
        }
        m_LRSN.erase(it);
        m_LRSN.push_back(std::make_pair(hash(socket->getSocketID()), control_info[0]));
        free(final_packet);
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
          char *s_buffer = (char *)malloc(MAXSIZE*sizeof(char));
          int p_size = packet[i]->getPayload(s_buffer, MAXSIZE);
          myfile << s_buffer;
          free(s_buffer);
        }
      }
      myfile.close();
    }

    else if (packets < 20 && packets > 0)
    {
      if (packet[last]->getfuncField() == 1)
      {
        std::vector< std::pair <uint64_t, uint32_t> >::iterator it;
        it = std::find_if(m_LRSN.begin(), m_LRSN.end(), CompareFirstInt(hash(*socketID)));
        if (it != m_LRSN.end())
        {
          uint32_t *last_seq = (uint32_t *)malloc(sizeof(uint32_t));
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
            control_info[0] = packet[19]->getSequence();             // last data packet sequence found

            c_packet->setType(type);
            c_packet->setExtendedType(etype);
            c_packet->setSubsequence(subseq);
            c_packet->setControlInfo(ACK, control_info);
            c_packet->setTimestamp(timestamp);
            c_packet->setSocketID(socketID);
            char *final_packet = (char *)malloc(40*sizeof(char));
            int length = c_packet->makePacket(final_packet);
            int _step = 0;
            while ( (socket->SendPacket(peer, final_packet, length) == -1) && (_step++ < 20));
            if (_step >= 20)
            {
              std::cerr << "ERROR-COULDN'T SEND ACK" << std::endl;
              return -1;
            }
            m_LRSN.erase(it);
            m_LRSN.push_back(std::make_pair(hash(socket->getSocketID()), control_info[0]));
            free(final_packet);
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
              char *s_buffer = (char *)malloc(MAXSIZE*sizeof(char));
              int p_size = packet[i]->getPayload(s_buffer, MAXSIZE);
              myfile << s_buffer;
              free(s_buffer);
            }
          }
          myfile.close();
        }
        break;
      }
      else if (packet[last]->getfuncField() != 1)
      {
        std::vector< std::pair <uint64_t, uint32_t> >::iterator it;
        it = std::find_if(m_LRSN.begin(), m_LRSN.end(), CompareFirstInt(hash(*socketID)));
        if (it != m_LRSN.end())
        {
          uint32_t *last_seq = (uint32_t *)malloc(sizeof(uint32_t));
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

            char *final_packet = (char *)malloc(40*sizeof(char));
            int length = c_packet->makePacket(final_packet);
            int _step = 0;
            while ( (socket->SendPacket(peer, final_packet, length) == -1) && (_step++ < 20));
            if (_step >= 20)
            {
              std::cerr << "ERROR-COULDN'T SEND ACK" << std::endl;
              return -1;
            }
            free(final_packet);
          }
        }
      }
    }
  }
  std::fstream myfile;
  myfile.open("write-in.bin", std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app);
  std::streampos _size = myfile.tellg();
  if (length < _size)
  {
    std::cerr << "ERROR-BUFFER DOESN'T CONTAIN ENOUGH MEMORY" << std::endl;
    return -1;
  }
  myfile.seekg(0, std::ios::beg);
  myfile.read(data, _size);
  myfile.close();

  free(type);
  free(etype);
  free(subseq);
  free(timestamp);
  free(socketID);
  for (int i = 0; i < 6; i++)
    free(control_info+i);
  free(c_packet);
  return (int)_size;
}
