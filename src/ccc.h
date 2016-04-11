enum Role {CLIENT = 0, SERVER = 1, RENDEZVOUS = 2}

#include

clas CCC
{
private:
  struct sockaddr_in *m_peer;
  Socket *m_socket;

public:
  CCC(const struct sockaddr_in peer, Socket socket);
  int init(Role eRole);
  int onACK();
  int onPacketSent();
  int onPacketReceived();
};
