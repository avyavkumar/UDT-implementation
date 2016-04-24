#include "core.h"

int main()
{
  UDTSocket *socket = UDTCore::open(AF_INET, 1234);
  // struct sockaddr_in *serverAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  // serverAddr->sin_family = AF_INET;
  // serverAddr->sin_port = htons(1234);
  // serverAddr->sin_addr.s_addr = INADDR_ANY;
  // memset(serverAddr->sin_zero, '\0', sizeof (serverAddr->sin_zero));
  // UDTCore::connect(socket,serverAddr);
  struct sockaddr_in *peer = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  while (1)
  {
    UDTCore::listen(socket, peer);
    if (UDTCore::m_recPackets.size() != 0)
      std::cout << "gotten" << std::endl;
  }

  // UDTSocket *socket = new UDTSocket();
  // socket->setIPVersion(IPv4);
  // socket->newSocket(AF_INET, 2346);
  // socket->bindSocket(2346);
  // char *buffer = (char*)malloc((MAXSIZE+50)*sizeof(char));
  // int length = socket->ReceivePacket(buffer);
  //
  // // struct sockaddr_in serverAddr;
  // // serverAddr.sin_family = AF_INET;
  // // serverAddr.sin_port = htons(2346);
  // // serverAddr.sin_addr.s_addr = INADDR_ANY;
  // // memset(serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));
  // // socket->SendPacket(serverAddr,buffer);
  // std::cout << "BYTES RECVD " << length << std::endl;
  // ControlPacket *packet = new ControlPacket();
  // uint32_t *type = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *etype = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *sub = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *t = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *a = (uint32_t *)malloc(6*sizeof(uint32_t));
  // // char *m_payload = (char *)malloc(MAXSIZE*sizeof(char));
  // // char *final_packet = (char *)malloc(length*sizeof(char));
  // // strcpy(m_payload, "testing");
  // // DataPacket *packet = new DataPacket();
  // // uint32_t *m_sequence = (uint32_t *)malloc(sizeof(uint32_t));
  // // uint32_t *m_funcField = (uint32_t *)malloc(sizeof(uint32_t));
  // // uint32_t *m_orderBit = (uint32_t *)malloc(sizeof(uint32_t));
  // // uint32_t *m_message = (uint32_t *)malloc(sizeof(uint32_t));
  // // uint32_t *m_timestamp = (uint32_t *)malloc(sizeof(uint32_t));
  // // uint32_t *m_socketID = (uint32_t *)malloc(sizeof(uint32_t));
  // // *m_sequence = 127;
  // // *m_funcField = 2;
  // // *m_orderBit = 1;
  // // *m_message = 282;
  // // *m_timestamp = 156;
  // packet->extractPacket(buffer);
  // int size = 0;
  // std::cout << packet->getType() << std::endl;
  // std::cout << packet->getExtendedType() << std::endl;
  // std::cout << packet->getSubsequence() << std::endl;
  // std::cout << packet->getTimestamp() << std::endl;
  // std::cout << packet->getSocketID() << std::endl;
  // int tempi = packet->getControlInfo(a, size);
  // std::cout << size << std::endl;
  // for(int i = 0; i < size; i++)
  //   std::cout << *(a+i) << std::endl;
  // // std::cout << packet->getPayload() << std::endl;
  // // std::cout << packet->getSequence() << std::endl;
  // // std::cout << packet->getfuncField() << std::endl;
  // // std::cout << packet->getOrderBit() << std::endl;
  // // std::cout << packet->getMessage() << std::endl;
  // // std::cout << packet->getTimestamp() << std::endl;
  // // std::cout << packet->getSocketID() << std::endl;
  //
  // // int temp = packet->makePacket(final_packet);

}
