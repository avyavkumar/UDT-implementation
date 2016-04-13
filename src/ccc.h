#include "packet.h"
#include "connection.h"
#include "handshakepacket.h"
#include "socket.h"
#include <stdlib.h>
#include <iomanip>
#include <chrono>
#include <ctime>

enum Role {CLIENT = 0, SERVER = 1, RENDEZVOUS = 2}

clas CCC
{
private:
  struct sockaddr_in m_peer;
  Socket *m_socket;
  int sendPossible;
  int recvPossible;

public:
  CCC(const struct sockaddr_in peer, Socket socket);
  int init(Role eRole);
  int onACK();
  int onPacketSent();
  int onPacketReceived();
};
