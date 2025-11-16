#ifndef _ROOMMANAGER_H
#define _ROOMMANAGER_H

#include "SOCKETINFO.h"

class RoomManager
{
	static RoomManager* instance;

	std::atomic<int> nextID;
	IOCPSessionManager& allClients;

	RoomManager() :
		nextID(0),
		allClients(IOCPSessionManager::getInstance())
	{}
public:
	static RoomManager& getInstance()
	{
		if (instance == nullptr) instance = new RoomManager;
		return *instance;
	}
	
	Room* GetRoom(int num = -1);
};


#endif
