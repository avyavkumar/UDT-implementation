enum Role {CLIENT = 0, SERVER = 1, RENDEZVOUS = 2}

#include "packet.h"

clas CCC
{
private:
  struct sockaddr_in *m_peer;
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
