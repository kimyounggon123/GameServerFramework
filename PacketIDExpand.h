#ifndef _PACKETIDEX_H
#define _PACKETIDEX_H
#include "PacketID.h"

// Headers
struct PacketTypeGame
{
	static constexpr int32_t Base = 100;
	static int count;
	static int Next() { return Base + count++; }

	static const int32_t Hello;
	static const int32_t Move;
	static const int32_t FireBullet;
	static const int32_t Die;
	static const int32_t Resurrect;
};


#endif