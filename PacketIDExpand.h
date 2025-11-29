#ifndef _PACKETIDEX_H
#define _PACKETIDEX_H
#include "PacketID.h"

// Headers
struct PacketTypeGame
{
	static constexpr int32_t Base = 1000;
	static constexpr int32_t Default = Base + 0;
	static constexpr int32_t ServerIsClosed = Base + 1;
};


#endif