g++ -c controlpacket.cpp -o controlpacket.o
g++ -c datapacket.cpp -o datapacket.o
g++ -c socket.cpp -o socket.o
g++ -c packet.cpp -o packet.o
g++ -c core.cpp -o core.o -std=c++11
# g++ -c sendpacket.cpp -o sendpacket.o -std=c++11
# g++ -c recvpacket.cpp -o recvpacket.o -std=c++11
# g++ core.o packet.o controlpacket.o datapacket.o socket.o sendpacket.o -o sender
# g++ core.o packet.o controlpacket.o datapacket.o socket.o recvpacket.o -o receiver
