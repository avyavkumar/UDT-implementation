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
