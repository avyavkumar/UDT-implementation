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

/****************************************************************************/
/*                            UDTSocket()                                   */
/*                            Constructor                                   */
/****************************************************************************/

UDTSocket::UDTSocket():
m_socketid(0),
m_address(),
m_storageSent(),
m_IPVersion(ERR),
m_storageRecv(),
m_port(0) {}

/****************************************************************************/
/*                            UDTSocket()                                   */
/*                            Destructor                                    */
/****************************************************************************/

UDTSocket::~UDTSocket()
{}

int UDTSocket::setIPVersion(IPVersion version)
{
  if (version != ERR)
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

int UDTSocket::SendPacket(const struct sockaddr_in peer, char *buffer)
{
  // TODO - Streamline with both IPv4 and IPv6
  socklen_t addr_size = sizeof(peer);
  int nBytes;
  try
  {
    nBytes = sendto(m_socketid,buffer,strlen(buffer),0,(struct sockaddr *)&peer,addr_size);
    if (nBytes == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-SEND() "<<e.what()<<std::endl;
    return -1;
  }

  // maintain bookkeeping data of the sent data
  // if (std::find(m_storageSent.begin(), m_storageSent.end(), peer) != m_storageSent.end())
  //   m_storageSent.push_back(peer);

  return nBytes;
}

/****************************************************************************/
/*                            ReceivePacket()                               */
/*               Receive data from any packet at a socket                   */
/*            Returns -1 if unsuccessful; bytes received if successful      */
/****************************************************************************/

int UDTSocket::ReceivePacket(char *buffer)
{
  // TODO - Streamline with both IPv4 and IPv6
  struct sockaddr_in client_address;
  socklen_t addr_size = sizeof(client_address);
  int nBytes;
  try
  {
    struct timeval timeout;
    timeout.tv_sec = 10;                                    // timeout after 10 seconds
    timeout.tv_usec = 0;
    setsockopt(m_socketid, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));
    nBytes = recvfrom(m_socketid,buffer,MAXSIZE,0,(struct sockaddr *)&client_address, &addr_size);
    if (nBytes == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-RECEIVE() "<<e.what()<<std::endl;
    return -1;
  }

  return nBytes;
  // maintain bookkeeping data of the received data

  // if (std::find(m_storageRecv.begin(), m_storageRecv.end(), client_address) != m_storageRecv.end())
  //   m_storageRecv.push_back(client_address);
}
