UDTCore::UDTCore():
{
  current_socket = (uint32_t *)malloc(sizeof(uint32_t));
}

/****************************************************************************/
/*                                  open()                                  */
/*            Returns a pointer to UDTSocket if sucessful binding           */
/*                              NULL otherwise                              */
/****************************************************************************/

UDTSocket* UDTCore::open(int conn_type, int port)
{
  UDT *socket = new Socket();
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
/****************************************************************************/

void connect(UDTSocket *socket, const sockaddr* peer);
{
  if (!socket || !peer)
  {
    std::cerr << "SOCKET/PEER ARE UNALLOC" << std::endl;
    return;
  }
  // intialise HANDSHAKEing information

  (*current_socket)++;
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
  *subsequence = // initial random subsequence
  *timestamp = // std::clock(); // timer mechanism
  *socketID = socket->getSocketID();

  controlInfo[0] = 4                                              // HS - UDT version
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
  _temp = setExtendedType(extendedtype);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-EXTENDEDTYPE" << std::endl;
    return;
  }
  _temp = setSubsequence(subsequence);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-SUBSEQUENCE" << std::endl;
    return;
  }
  _temp = setTimestamp(timestamp);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-TIMESTAMP" << std::endl;
    return;
  }
  _temp = setControlInfo(HANDSHAKE, controlInfo);
  if (_temp == -1)
  {
    std::cerr << "FAILURE-CONTROLINFO" << std::endl;
    return;
  }
  _temp = setSocketID(socketID);
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
  do
  {
    // send the HANDSHAKEing packet
    int _sent = packet->SendPacket(peer, s_packet, _bytes);
    if (_sent != 40)
    {
      std::cerr << "ERR-CORRUPT BYTES" << std::endl;
      return;
    }
    // intialise the timer
    std::clock_t c_start = std::clock();
    while (std::clock() - c_start < 1000);

    // Wait for the return - timer version
    int _recv = packet->ReceivePacket(r_packet);
    // 40 bytes for control packet; acts as a check
    if (_recv == 40)
    {
      // get the flag bit of the packet; check if control bit
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
              temp = std::make_pair(hash(rec_packet->getSocketID()), control_info[0]);
              m_Version.push_back(temp);

              // HS - UDT socket type
              temp = std::make_pair(hash(rec_packet->getSocketID()), control_info[1]);
              m_Type.push_back(temp);

              // HS - random initial sequence number
              temp = std::make_pair(hash(rec_packet->getSocketID()), control_info[2]);
              m_ISN.push_back(temp);

              // HS - maximum segment size
              temp = std::make_pair(hash(rec_packet->getSocketID()), control_info[3]);
              m_MSS.push_back(temp);

              // HS - flow control window size
              temp = std::make_pair(hash(rec_packet->getSocketID()), control_info[4]);
              m_FlightFlagSize.push_back(temp);

              _successConnect = 1;
              break;
            }
          }
        }
      }
    }
    _step++;
  } while (_step < 20);                         // try this 20 times

  if (_successConnect == 0)
    std::cerr << "ERROR-CONNECTION" << std::endl;

  free(packet);
  free(rec_packet);
}

uint32_t UDTCore::hash(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}
