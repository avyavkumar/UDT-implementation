#include "core.h"

int main()
{
  UDTSocket *socket = UDTCore::open(AF_INET, 6789);
  struct sockaddr_in *serverAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  serverAddr->sin_family = AF_INET;
  serverAddr->sin_port = htons(1234);
  serverAddr->sin_addr.s_addr = INADDR_ANY;
  memset(serverAddr->sin_zero, '\0', sizeof (serverAddr->sin_zero));
  if (!socket)
    std::cerr << "NULL" << std::endl;
  UDTCore::connect(socket,serverAddr);
  char *buffer = (char *)malloc(1000*sizeof(char));
  for (long long int i = 0; i < 1000; i++)
    buffer[i] = i%10;
  UDTCore::send(socket, *serverAddr, buffer, 1000);
  UDTCore::close(socket, serverAddr);

  // ControlPacket *packet = new ControlPacket();
  // uint32_t *type = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *etype = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *sub = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *t = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *a = (uint32_t *)malloc(6*sizeof(uint32_t));
  // uint32_t *z = (uint32_t *)malloc(sizeof(uint32_t));
  // *type = 3;
  // *etype = 0x12;
  // *sub = 45;
  // *t = 82372;
  // for (int i = 0; i < 6; i++)
  //   *(a+i) = 7832+i;
  // *z = 3414;
  // int size = 0;
  // int tempi;
  // packet->setType(type);
  // packet->setExtendedType(etype);
  // packet->setSubsequence(sub);
  // packet->setControlInfo(NAK, a);
  // packet->setTimestamp(t);
  // packet->setSocketID(z);
  // std::cout << packet->getType() << std::endl;
  // std::cout << packet->getExtendedType() << std::endl;
  // std::cout << packet->getSubsequence() << std::endl;
  // std::cout << packet->getTimestamp() << std::endl;
  // std::cout << packet->getSocketID() << std::endl;
  // tempi = packet->getControlInfo(a, size);
  // // std::cout << size << std::endl;
  // for(int i = 0; i < size; i++)
  //   std::cout << *(a+i) << std::endl;
  // //std::cout << packet->getType() << std::endl;
  // char *s_packet = (char *)malloc(40*sizeof(char));
  // int temp = packet->makePacket(s_packet);
  // std::cout << "type" << std::endl;
  // std::cout << getFlag(s_packet, temp) << std::endl;
  // char *buffer = (char*)malloc((MAXSIZE+50)*sizeof(char));
  // for (int i = 0; i < 40; i++)
  //   std::cout << s_packet[i] << std::endl;
  // ControlPacket *p_packet = new ControlPacket();
  // std::cout << "p_packet" << std::endl;
  // p_packet->extractPacket(s_packet);
  // std::cout << p_packet->getType() << std::endl;
  // std::cout << p_packet->getExtendedType() << std::endl;
  // std::cout << p_packet->getSubsequence() << std::endl;
  // std::cout << p_packet->getTimestamp() << std::endl;
  // std::cout << p_packet->getSocketID() << std::endl;
  // tempi = p_packet->getControlInfo(a, size);
  // std::cout << size << std::endl;
  // for(int i = 0; i < size; i++)
  //   std::cout << *(a+i) << std::endl;
  // DataPacket *packet = new DataPacket();
  // uint32_t *m_sequence = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *m_funcField = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *m_orderBit = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *m_socketID = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *m_message = (uint32_t *)malloc(sizeof(uint32_t));
  // uint32_t *m_timestamp = (uint32_t *)malloc(sizeof(uint32_t));
  // char *m_payload = (char *)malloc(50*sizeof(char));
  // char *final_packet = (char *)malloc(62*sizeof(char));
  // strcpy(m_payload, "testing");
  //
  // *m_sequence = 127;
  // *m_funcField = 2;
  // *m_orderBit = 1;
  // *m_message = 282;
  // *m_timestamp = 156;
  // *m_socketID = 213;
  //
  // packet->setPayload(m_payload, strlen("testing"));
  // packet->setSequence(m_sequence);
  // packet->setMessage(m_message);
  // packet->setTimestamp(m_timestamp);
  // packet->setfuncField(m_funcField);
  // packet->setOrderBit(m_orderBit);
  // packet->setSocketID(m_socketID);
  //
  // int temp = packet->makePacket(final_packet);
  // // std::cout << temp << std::endl;
  // // for (int i = 0; i < temp; i++)
  // //   std::cout << final_packet[i] << std::endl;
  //

  // UDTSocket *socket = new UDTSocket();
  // socket->newSocket(AF_INET, 72);
  //
  // struct sockaddr_in serverAddr;
  // serverAddr.sin_family = AF_INET;
  // serverAddr.sin_port = htons(2346);
  // serverAddr.sin_addr.s_addr = INADDR_ANY;
  // memset(serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));
  // std::cout << "BYTES SENT "<< socket->SendPacket(serverAddr,s_packet, 40) << std::endl;

}
