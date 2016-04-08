enum IPVersion {ERR=0,IPv4, IPv6};

class UDTSocket
{
protected:
   uint64_t m_socketid;                                 // socket ID - needs to be set manually
   struct sockaddr_in m_address;                        // struct containing info about socket address
   IPVersion m_IPVersion;                               // IP version
   std::vector<struct sockaddr_storage> m_storageSent;  // vector of all the connections made
   std::vector<struct sockaddr_storage> m_storageRecv;  // vector of all the connections made
   socklen_t m_addrlen;                                 // length of the struct
   uint32_t m_port;                                     // port

public:
  UDTSocket();
  ~UDTSocket();
  int setIPVersion(IPVersion version);
  int newSocket(int af, int type);
  int SendPacket(const struct sockaddr peer, char *buffer);
  int ReceivePacket(char *buffer);
};
