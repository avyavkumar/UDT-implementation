// Functionality:
//    initialize a UDT entity and bind to a local address.
// Parameters:
//    None.
// Returned value:
//    None.

UDTCore::open(UDTSocket *socket)
{
  if (!socket)
  {
    std::cerr << "UNAVAILABLE MEMORY FOR SOCKET" << std::endl;
    return -1;
  }
  // intialise HANDSHAKEing information
  // send the HANDSHAKEing packet
  // Wait for the return 

}
