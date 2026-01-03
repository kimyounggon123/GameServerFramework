#ifndef _ROOMMANAGER_H
#define _ROOMMANAGER_H

#include "FPSRoom.h"
class RoomManager
{
	static RoomManager* instance;
	bool isInitialized;

	std::atomic<int> nextID;
	
	IOCPSessionManager& allClients;


	RoomManager() :
		nextID(0), isInitialized(false),
		allClients(IOCPSessionManager::getInstance())
	{}
public:
	~RoomManager()
	{ 
		allClients.deleteUDPSOCKET();
	}
	static RoomManager& getInstance()
	{
		if (instance == nullptr) instance = new RoomManager;
		return *instance;
	}
	
	bool Initialize();

	bool InputRoom(Room* room);
	bool DeleteRoom(int ID);

	Room* GetRoom(int ID = 0);
};


#endif
