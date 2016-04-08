//#include "socket.h"
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

#define MAXSIZE 100

enum IPVersion {ERR=0,IPv4, IPv6};

class UDTSocket
{
protected:
   uint64_t m_socketid;                                  // socket ID - needs to be set manually
   struct sockaddr_in m_address;                         // struct containing info about socket address
   IPVersion m_IPVersion;                                // IP version
   std::vector<struct sockaddr*> m_storageSent;          // vector of all the connections made
   std::vector<struct sockaddr_storage*> m_storageRecv;  // vector of all the connections made
   uint32_t m_port;                                      // port

public:
  UDTSocket();
  ~UDTSocket();
  int setIPVersion(IPVersion version);
  int newSocket(int type);
  int bindSocket(int port);
  int SendPacket(const struct sockaddr_in peer, char *buffer);
  int ReceivePacket(char *buffer);
};

UDTSocket::UDTSocket():
m_socketid(0),
m_address(),
m_storageSent(),
m_IPVersion(ERR),
m_storageRecv(),
m_port(0) {}

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
    m_socketid = socket(AF_INET, SOCK_DGRAM, 0);
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

int UDTSocket::SendPacket(const struct sockaddr_in peer, char *buffer)
{
  // TODO - Streamline with both IPv4 and IPv6
  socklen_t addr_size = sizeof(peer);
  try
  {
    int nBytes = sendto(m_socketid,buffer,strlen(buffer),0,(struct sockaddr *)&peer,addr_size);
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

  return 0;
}

int UDTSocket::ReceivePacket(char *buffer)
{
  // TODO - Streamline with both IPv4 and IPv6
  struct sockaddr_storage client_address;
  socklen_t addr_size = sizeof(client_address);
  try
  {
    int nBytes = recvfrom(m_socketid,buffer,MAXSIZE,0,(struct sockaddr *)&client_address, &addr_size);
    if (nBytes == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-RECEIVE() "<<e.what()<<std::endl;
    return -1;
  }

  // maintain bookkeeping data of the received data

  // if (std::find(m_storageRecv.begin(), m_storageRecv.end(), client_address) != m_storageRecv.end())
  //   m_storageRecv.push_back(client_address);

  return 0;
}

int main()
{
  UDTSocket *socket = new UDTSocket();
  socket->setIPVersion(IPv4);
  socket->newSocket(12);
  char *buffer = (char *)malloc(MAXSIZE*sizeof(char));
  strcpy(buffer,"checking_1");
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(2346);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  memset(serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));
  socket->SendPacket(serverAddr,buffer);
  strcpy(buffer,"checking_2");
  socket->SendPacket(serverAddr,buffer);
  strcpy(buffer,"checking_3");
  socket->SendPacket(serverAddr,buffer);

  return 0;
}