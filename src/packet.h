// types of packets
enum PacketType {DATA = 0x0, CONTROL = 0x8, ERROR};

// types of subcategories in control packets
enum ControlPacketType {HANDSHAKE, ACK, ACK2, NAK, KeepAlive, ShutDown, DropRequest, ERR, UNKNOWN, CONGESTION};

#define MAXSIZE 508
