#include "packet.h"

/****************************************************************************/
/*                              getFlag()                                   */
/*                        Returns the type of flag                          */
/****************************************************************************/

PacketType getFlag(char *packet, int length)
{
  if (!packet)
    return ERROR;
  else if (((packet[39] >> 7) & 0x1 == 1) && length == 40)
    return CONTROL;
  else if (((packet[39] >> 7) & 0x1 == 0) && length == 40)
    return DATA;
}
