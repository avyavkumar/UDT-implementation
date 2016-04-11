// get handshaking packet
// check with the request packet
#include "connection.h"
#include "handshakepacket.h"
#include <stdlib.h>
#include <iomanip>
#include <chrono>
#include <ctime>

int CCC::init(Role eRole)
{
  if (eRole == CLIENT)
  {
    HandShakePacket *m_hspacket = new HandShakePacket();
    do
    {
      // send handshaking packet
      char *buffer = (char *)malloc(100*sizeof(char));
      int err_code = m_hspacket->makePacket(buffer);
      if (err_code == -1)
      {
        std::cerr << "CLIENT/ERROR:MAKEPACKET" << std::endl;
        return -1;
      }
      try
      {
        err_code = m_socket->SendPacket(m_peer, buffer);
        if (err_code == -1)
          throw std::exception();
        std::clock_t c_start = std::clock();
      }
      catch(std::exception &e)
      {
        std::cerr <<"CLIENT/ERROR:HANDSHAKE-SEND"<<e.what()<<std::endl;
        return -1;
      }
      // wait for some time to check if there is a response packet
      while (std::clock() - c_start < 1000);
      err_code = m_socket->ReceivePacket(buffer);
      if (err_code != -1)
        break;
    } while(1);
    // If packet is handshaking type and contains relevant stuff
    // sendPossible++
    // recvPossible++
  }
  else if (eRole == SERVER)
  {

  }
  else
  {

  }
}
