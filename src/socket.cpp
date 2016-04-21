#include "socket.h"

/****************************************************************************/
/*                            UDTSocket()                                   */
/*                            Constructor                                   */
/****************************************************************************/

UDTSocket::UDTSocket():
m_socketid(0),
m_address(),
m_storageSent(),
m_IPVersion(Err),
m_storageRecv(),
m_family(-1),
m_port(0) {}

/****************************************************************************/
/*                            UDTSocket()                                   */
/*                            Destructor                                    */
/****************************************************************************/

UDTSocket::~UDTSocket()
{}

int UDTSocket::setIPVersion(IPVersion version)
{
  if (version != Err)
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
  m_family = family;
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

int UDTSocket::SendPacket(const struct sockaddr_in peer, char *buffer, int length)
{
  // TODO - Streamline with both IPv4 and IPv6
  socklen_t addr_size = sizeof(peer);
  int nBytes;
  try
  {
    nBytes = sendto(m_socketid,buffer,length,0,(struct sockaddr *)&peer,addr_size);
    if (nBytes == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-SEND() "<<e.what()<<std::endl;
    return -1;
  }

  return nBytes;
}

/****************************************************************************/
/*                            ReceivePacket()                               */
/*               Receive data from any packet at a socket                   */
/*            Returns -1 if unsuccessful; bytes received if successful      */
/****************************************************************************/

int UDTSocket::ReceivePacket(char *buffer, struct sockaddr_in *client_address)
{
  // TODO - Streamline with both IPv4 and IPv6
  socklen_t addr_size = sizeof(sockaddr_in);
  int nBytes;
  try
  {
    struct timeval timeout;
    timeout.tv_sec = 10;                                    // timeout after 10 seconds
    timeout.tv_usec = 0;
    setsockopt(m_socketid, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));
    nBytes = recvfrom(m_socketid,buffer,MAXSIZE,0,(struct sockaddr *)client_address, &addr_size);
    if (nBytes == -1)
      throw std::exception();
  }

  catch(std::exception &e)
  {
    std::cerr <<"ERROR-RECEIVE() "<<e.what()<<std::endl;
    return -1;
  }

  return nBytes;
}

/****************************************************************************/
/*                             getSocketID()                                */
/*         Returns 0 if unsuccessful; socketID received if successful       */
/****************************************************************************/

uint64_t UDTSocket::getSocketID()
{
    return m_socketid;
}

/****************************************************************************/
/*                             getFamily()                                  */
/*        Returns -1 if unsuccessful;getFamily received if successful       */
/****************************************************************************/

int UDTSocket::getFamily()
{
    return m_family;
}
