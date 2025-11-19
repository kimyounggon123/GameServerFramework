#ifndef _ROOMMANAGER_H
#define _ROOMMANAGER_H

#include "SOCKETINFO.h"

class RoomExpand : public Room
{
	ThreadSafeQueue<Packet*> broadcastQueue;
public:
	RoomExpand(): Room(), broadcastQueue(false)
	{ }
	bool EnqueueBroadcast(Packet*& input)
	{
		return broadcastQueue.enqueue(input);
	}
	bool DequeueBroadcast(Packet*& output)
	{
		return broadcastQueue.dequeue(output);
	}
};



class RoomManager
{
	static RoomManager* instance;
	std::atomic<int> nextID;

	ThreadSafeQueue<RoomExpand*> broadcastLine;

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
