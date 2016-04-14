// types of packets
#ifndef PACKET_TYPE
#define PACKET_TYPE
enum PacketType {DATA = 0x0, CONTROL = 0x1, ERROR};
#endif

// types of subcategories in control packets
#ifndef CONTROL_PACKET_TYPE
#define CONTROL_PACKET_TYPE
enum ControlPacketType {HANDSHAKE, ACK, ACK2, NAK, KeepAlive, ShutDown, DropRequest, ERR, UNKNOWN, CONGESTION};
#endif

#ifndef MAXSIZE
#define MAXSIZE 508
#endif

PacketType getFlag(char *packet, int length);
