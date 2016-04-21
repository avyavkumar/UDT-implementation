#include <sys/types.h>
#include <ifaddrs.h>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef MAXSIZE
#define MAXSIZE 508
#endif

#ifndef IP_VERSION
#define IP_VERSION
enum IPVersion {Err=0,IPv4, IPv6};
#endif

// types of packets
#ifndef PACKET_TYPE
#define PACKET_TYPE
enum PacketType {DATA = 0x0, CONTROL = 0x1, ERROR};
#endif

// types of subcategories in control packets
#ifndef CONTROL_PACKET_TYPE
#define CONTROL_PACKET_TYPE
enum ControlPacketType {HANDSHAKE = 0, ACK = 2, ACK2 = 6, NAK = 3, KeepAlive = 1, ShutDown = 5,
                        DropRequest = 7, ERR, UNKNOWN, CONGESTION};
#endif

PacketType getFlag(char *packet, int length);

PacketType getFlag(char *packet, int length)
{
  if (!packet)
    return ERROR;
  else if (((packet[39] >> 7) & 0x1 == 1) && length == 40)
    return CONTROL;
  else if (((packet[39] >> 7) & 0x1 == 0) && length == 40)
    return DATA;
}

class UDTSocket
{
protected:
   uint64_t m_socketid;
   int m_family;
   struct sockaddr_in m_address;                         // struct containing info about socket address
   IPVersion m_IPVersion;                                // IP version
   std::vector<struct sockaddr*> m_storageSent;          // vector of all the connections made
   std::vector<struct sockaddr_storage*> m_storageRecv;  // vector of all the connections made
   uint32_t m_port;                                      // port

public:
  UDTSocket();
  ~UDTSocket();
  int setIPVersion(IPVersion version);
  int newSocket(int family, int port);
  int SendPacket(const struct sockaddr_in peer, char *buffer, int length);
  int bindSocket(int port);
  int ReceivePacket(char *buffer, struct sockaddr_in *peer);
  uint64_t getSocketID();
  int getFamily();
};

/****************************************************************************/
/*                            UDTSocket()                                   */
/*                            Constructor                                   */
/****************************************************************************/

UDTSocket::UDTSocket():
m_socketid(0),
m_address(),
m_storageSent(),
m_IPVersion(Err),
m_storageRecv(),
m_family(-1),
m_port(0) {}

/****************************************************************************/
/*                            UDTSocket()                                   */
/*                            Destructor                                    */
/****************************************************************************/

UDTSocket::~UDTSocket()
{}

int UDTSocket::setIPVersion(IPVersion version)
{
  if (version != Err)
  {
    m_IPVersion = version;
    return 1;
  }
  else
    return -1;
}

/****************************************************************************/
/*                            newSocket()                                   */
/*         Initialise the socket interface at a specified port              */
/*            Returns -1 if unsuccessful; 0 if successful                   */
/****************************************************************************/

int UDTSocket::newSocket(int family, int port)
{
  m_port = port;
  /*Create UDP socket*/
  m_family = family;
  try
  {
    m_socketid = socket(AF_INET, family, 0);
    if (m_socketid == -1)
      throw std::exception();
  }
  catch(std::exception &e)
  {
    std::cerr <<"ERROR-SOCKET() "<<e.what()<<std::endl;
    m_socketid = 0;
    return -1;
  }
  /*Configure settings in address struct*/
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(port);
  // the s_addr is set to INADDR_ANY to ensure that all IPs corresponding to the machine are subjected to bind()
  m_address.sin_addr.s_addr = INADDR_ANY;
  memset(m_address.sin_zero, '\0', sizeof(m_address.sin_zero));
  return 0;
}

/****************************************************************************/
/*                            bindSocket()                                  */
/*             Bind the socket interface at a specified port                */
/*             Returns -1 if unsuccessful; 0 if successful                  */
/****************************************************************************/

int UDTSocket::bindSocket(int port)
{
  socklen_t size = sizeof(m_address);
  /*Bind socket with address struct*/
  try
  {
    int temp = bind(m_socketid, (struct sockaddr *) &m_address, size);
    if (temp == -1)
      throw std::exception();
  }
  catch(std::exception &e)
  {
    std::cerr <<"ERROR-BIND() "<<e.what()<<std::endl;
    m_socketid = 0;
    return -1;
  }
  return 0;
}

/****************************************************************************/
/*                            SendPacket()                                  */
/*               Send data to a specified peer address                      */
/*       Returns -1 if unsuccessful; number of bytes sent if successful     */
/****************************************************************************/

int UDTSocket::SendPacket(const struct sockaddr_in peer, char *buffer, int length)
{
  // TODO - Streamline with both IPv4 and IPv6
  socklen_t addr_size = sizeof(peer);
  int nBytes;
  try
  {
    nBytes = sendto(m_socketid,buffer,length,0,(struct sockaddr *)&peer,addr_size);
    if (nBytes == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-SEND() "<<e.what()<<std::endl;
    return -1;
  }

  return nBytes;
}

/****************************************************************************/
/*                            ReceivePacket()                               */
/*               Receive data from any packet at a socket                   */
/*            Returns -1 if unsuccessful; bytes received if successful      */
/****************************************************************************/

int UDTSocket::ReceivePacket(char *buffer, struct sockaddr_in *client_address)
{
  // TODO - Streamline with both IPv4 and IPv6
  socklen_t addr_size = sizeof(sockaddr_in);
  int nBytes;
  try
  {
    struct timeval timeout;
    timeout.tv_sec = 10;                                    // timeout after 2 seconds
    timeout.tv_usec = 0;
    setsockopt(m_socketid, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));
    nBytes = recvfrom(m_socketid,buffer,MAXSIZE,0,(struct sockaddr *)client_address, &addr_size);
    if (nBytes == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-RECEIVE() "<<e.what()<<std::endl;
    return -1;
  }

  return nBytes;
}

/****************************************************************************/
/*                             getSocketID()                                */
/*         Returns 0 if unsuccessful; socketID received if successful       */
/****************************************************************************/

uint64_t UDTSocket::getSocketID()
{
    return m_socketid;
}

/****************************************************************************/
/*                             getFamily()                                  */
/*        Returns -1 if unsuccessful;getFamily received if successful       */
/****************************************************************************/

int UDTSocket::getFamily()
{
    return m_family;
}

int main()
{
  UDTSocket *socket = new UDTSocket();
  socket->setIPVersion(IPv4);
  socket->newSocket(SOCK_DGRAM, 2346);
  socket->bindSocket(2346);
  char *buffer = (char *)malloc(MAXSIZE*sizeof(char));
  struct sockaddr_in *client = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  while (1)
  {
    int size = socket->ReceivePacket(buffer, client);
    std::cout << buffer << std::endl;
    if (client)
      std::cout << "YES" << std::endl;
  }

  return 0;
}
