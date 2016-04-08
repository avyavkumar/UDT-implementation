#include "socket.h"
#include <sys/types.h>
#include <ifaddrs.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

UDTSocket::UDTSocket():
m_socketid(0),
m_port(0),
m_address(0),
m_addrlen(0) {}

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

int UDTSocket::newSocket(int port)
{
  m_port = port;
  /*Create UDP socket*/
  try
  {
    if (IPVersion == IPv4)
      m_socketid = socket(AF_INET, SOCK_DGRAM, 0);
    else if (IPVersion == IPv6)
      m_socketid = socket(AF_INET6, SOCK_DGRAM, 0);
    if (m_socketid == -1)
      throw std::exception();
  }
  catch(std::exception &e)
  {
    std::cerr <<"ERROR-SOCKET() "<<e.what<<std::endl;
    m_socketid = 0;
    return -1;
  }

  /*Configure settings in address struct*/
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(port);
  // the s_addr is set to INADDR_ANY to ensure that all IPs corresponding to the machine are subjected to bind()
  m_address.sin_addr.s_addr = INADDR_ANY;
  memset(m_address.sin_zero, '\0', sizeof(m_address.sin_zero));

  /*Bind socket with address struct*/
  try
  {
    int temp = bind(m_address, (struct sockaddr *) &m_address, sizeof(m_address));
    if (temp == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-BIND() "<<e.what<<std::endl;
    m_socketid = 0;
    return -1;
  }
  return 0;
}

int UDTSocket::SendPacket(const struct sockaddr peer, char *buffer)
{
  // TODO - Streamline with both IPv4 and IPv6
  int addr_size = sizeof(client_address);
  try
  {
    int nBytes = sendto(m_socketid,buffer,sizeof(buffer),0,(struct sockaddr *)&peer,addr_size);
    if (nBytes == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-SEND() "<<e.what<<std::endl;
    return -1;
  }

  // maintain bookkeeping data of the sent data
  if (std::find(m_storageSent.begin(), m_storageSent.end(), client_address) != m_storageSent.end())
    m_storageSent.push_back(client_address);

  return 0;
}

int UDTSocket::ReceivePacket(char *buffer)
{
  // TODO - Streamline with both IPv4 and IPv6
  struct sockaddr_storage client_address;
  int addr_size = sizeof(client_address);
  try
  {
    int nBytes = recvfrom(m_address,buffer,sizeof(buffer),0,(struct sockaddr *)&client_address, &addr_size);
    if (nBytes == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-RECEIVE() "<<e.what<<std::endl;
    return -1;
  }

  // maintain bookkeeping data of the received data
  if (std::find(m_storageRecv.begin(), m_storageRecv.end(), client_address) != m_storageRecv.end())
    m_storageRecv.push_back(client_address);

  return 0;
}
