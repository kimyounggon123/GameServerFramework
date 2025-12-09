#ifndef _PACKETIDEX_H
#define _PACKETIDEX_H
#include "PacketID.h"

// Headers
struct PacketTypeGame
{
	static constexpr int32_t Base = 100;

	static constexpr int32_t Hello =		Base + 0;
	static constexpr int32_t Bye =			Base + 1;
	static constexpr int32_t Move =			Base + 2;
	static constexpr int32_t FireBullet =	Base + 3;
	static constexpr int32_t Die =			Base + 4;
	static constexpr int32_t Resurrect =	Base + 5;
};


#endif