#ifndef _PACKETIDEX_H
#define _PACKETIDEX_H
#include "PacketID.h"

// Headers
struct PacketTypeGame
{
	static constexpr int32_t Base = 1000;
	static int count;
	static int Next() { return count++; }

	static const int32_t Hello;
	static const int32_t FireBullet;
};


#endif