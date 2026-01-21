#ifndef _PACKETGAMEPROCESS_H
#define _PACKETGAMEPROCESS_H

#include "PacketProcess.h"
#include "RoomManager.h"
#include "PacketIDExpand.h"
#include "DispatcherHub.h"



class PacketGameProcess : public PacketProcess
{
	DispatcherHub& dispatcherHub;
	RoomManager& roomManager;

	bool BroadCastThis(Task& input, int RoomID = 0);
	bool DBThis(Task& input);

	bool Hello(Task& input);
	bool Bye(Task& input);
	bool Move(Task& input);
	bool FireBullet(Task& input);
	bool Die(Task& input);
	bool Resurrect(Task& input);

public:
	PacketGameProcess(): PacketProcess(), dispatcherHub(DispatcherHub::getInstance()), roomManager(RoomManager::getInstance())
	{}

	void initialize() override;
};

#endif