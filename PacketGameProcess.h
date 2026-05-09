#ifndef _PACKETGAMEPROCESS_H
#define _PACKETGAMEPROCESS_H

#include "Server/Module/PacketProcess.h"
#include "PacketIDExpand.h"


class PacketGameProcess : public PacketProcess
{

	bool Hello(Task& input);
	bool Bye(Task& input);
	bool Move(Task& input);
	bool FireBullet(Task& input);
	bool Die(Task& input);
	bool Resurrect(Task& input);

public:
	PacketGameProcess(): PacketProcess()
	{}

	void initialize() override;
};

#endif