#include "core.h"

uint32_t UDTCore::current_socket = 0;

struct CompareFirst
{
  CompareFirst(uint32_t val) : val_(val) {}
  bool operator()(const std::pair<uint32_t,sockaddr_in> &elem) const {
    return val_ == elem.first;
  }
  private:
    uint32_t val_;
};

uint32_t UDTCore::hash(uint32_t x)
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
  uint32_t *socketID = (uint32_t *)malloc(sizeof(uint32_t));
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
              uint32_t *socketID = (uint32_t *)malloc(sizeof(uint32_t));
              uint32_t *controlInfo = (uint32_t *)malloc(6*sizeof(uint32_t));
              *type = HANDSHAKE;
              *extendedtype = 0;
              *subsequence = 0x4324;     // initial random subsequence
              *timestamp = std::clock(); // timer mechanism
              *socketID = socket->getSocketID();

              controlInfo[0] = 4;                                              // HS - UDT version
              controlInfo[1] = socket->getFamily();                            // HS - UDT Socket type - 0 - and 1-
              controlInfo[2] = *subsequence;                                   // HS - random initial seq #
              if (control_info[3] > _currPacketSize)                           // HS - maximum segment size
                controlInfo[3] = _currPacketSize;
              else
              {
                controlInfo[3] = control_info[3];
                _currPacketSize = control_info[3];
              }
              if (control_info[4] > _currFlowWindowSize)                       // HS - flow control window size
                controlInfo[3] = _currFlowWindowSize;
              else
              {
                controlInfo[3] = control_info[3];
                _currFlowWindowSize = control_info[3];
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
  uint32_t *socketID = (uint32_t *)malloc(sizeof(uint32_t));
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
    // send the ShutDown packet
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
  std::vector< std::pair <uint32_t, sockaddr_in> >::iterator it;
  it = std::find_if(m_activeConn.begin(), m_activeConn.end(), CompareFirst(hash(peer->sin_addr.s_addr)));
  if (it != m_activeConn.end())
    m_activeConn.erase(it);
}

/****************************************************************************/
/*                                 send()                                   */
/*             Sends data to a destination contained by socket              */
/*  Assumes that the node has established UDT connection with destination   */
/****************************************************************************/

int UDTCore::send(UDTSocket *socket, const struct sockaddr_in peer, const char* data, int len)
{
  // send the data with the following in mind
  // packet loss
  // ACK received after 16 packets sent
  // if there is no ACK, then send them again
  // if NACK received, get the numbers of packets and send them again

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
  while (packets < total_packets)
  {
    for (int i = 0; i < 20; i++)
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
          packet[i]->setPayload(socket, data, len);
          char *final_packet = (char *)malloc((len+16)*sizeof(char));
          nBytes = packet[i]->makePacket(final_packet);
          if (nBytes != len + 16)
          {
            std::cerr << "ERROR-CORRUPT PACKET WITH SEQUENCE " <<packet->getSequence()<<" AND SOCKETID "<<packet->getSocketID() << std::endl;
            return;
          }
          socket->SendPacket(peer, final_packet, len+16);
          if (socket->SendPacket(peer, final_packet, final_length+16) != nBytes)
          {
            std::cerr << "ERROR-CORRUPT PACKET SENT WITH SEQUENCE " <<packet->getSequence()<<" AND SOCKETID "<<packet->getSocketID() << std::endl;
            return;
          }
        }
        else if (packets == total_packets - 1)
        {
          packet[i]->setPayload(socket, data, final_length);
          char *final_packet = (char *)malloc((final_length+16)*sizeof(char));
          nBytes = packet[i]->makePacket(final_packet);
          if (nBytes != final_length + 16)
          {
            std::cerr << "ERROR-CORRUPT PACKET WITH SEQUENCE " <<packet->getSequence()<<" AND SOCKETID "<<packet->getSocketID() << std::endl;
            return;
          }
          if (socket->SendPacket(peer, final_packet, final_length+16) != nBytes)
          {
            std::cerr << "ERROR-CORRUPT PACKET SENT WITH SEQUENCE " <<packet->getSequence()<<" AND SOCKETID "<<packet->getSocketID() << std::endl;
            return;
          }
        }
        else
        {
          packet[i]->setPayload(socket, data, len_init);
          char *final_packet = (char *)malloc((len_init+16)*sizeof(char));
          nBytes = packet[i]->makePacket(final_packet);
          if (nBytes != len_init + 16)
          {
            std::cerr << "ERROR-CORRUPT PACKET WITH SEQUENCE " <<packet->getSequence()<<" AND SOCKETID "<<packet->getSocketID() << std::endl;
            return;
          }
          socket->SendPacket(peer, final_packet, len_init+16);
          if (socket->SendPacket(peer, final_packet, final_length+16) != nBytes)
          {
            std::cerr << "ERROR-CORRUPT PACKET SENT WITH SEQUENCE " <<packet->getSequence()<<" AND SOCKETID "<<packet->getSocketID() << std::endl;
            return;
          }
        }
        packets++;
        curr_seq++;
      }
    }

    // Now that 20 packets have been sent, we wait for the response (ACK/NACK/no response)
    // In ACK - all the packets have arrived successfully, we prepare to send the next 20 data packets
    // In NACK - we get the sequence that couldn't be sent. We try again to send them, if still unsuccessful,
    // we place them in the loss list and move on after reporting an error
    // No response - we retransmit the packets

    while (std::clock() - c_start < 20000);                            // Wait for some time for a response
    char *recv_packet = (char *)malloc(40*sizeof(char));
    std::clock_t c_start = std::clock();
    int size = socket->ReceivePacket(recv_packet);
    ControlPacket *c_packet = new ControlPacket();
    if (c_packet->extractPacket(recv_packet) != 1)
      continue;
    else
    {
      // in order, check if the packet is control type; then check if the socketID matches and then check the ACK, NACK signals

    }

  }
}
