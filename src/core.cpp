#include "core.h"

uint32_t UDTCore::current_socket = 0;
int UDTCore::_successConnectServer = 0;
int UDTCore::_successConnectClient = 0;

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

              _successConnectClient = 1;
              m_activeConn.push_back(std::make_pair(UDTCore::hash(peer->sin_addr.s_addr), *peer));
              break;
            }
          }
        }
      }
    }
    _step++;
  } while (_step < 20);                         // try this 20 times

  if (_successConnectClient == 0)
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

            _successConnectServer = 1;

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
    _step++;
  } while (_step < 20);                         // try this 20 times

  if (_successConnectServer == 0)
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

  if (_successConnectClient == 0)
    std::cerr << "ERROR-COULD NOT SHUT CONNECTION" << std::endl;

  free(packet);
  free(s_packet);

  //remove the peer from the list of active connections
  std::vector< std::pair <uint32_t, sockaddr_in> >::iterator it;
  it = std::find_if(m_activeConn.begin(), m_activeConn.end(), CompareFirst(hash(peer->sin_addr.s_addr)));
  if (it != m_activeConn.end())
    m_activeConn.erase(it);
}
