#ifndef _ROOMMANAGER_H
#define _ROOMMANAGER_H

#include "SOCKETINFO.h"

class GameRoom : public Room
{
	/*
	* Room fields
	
	int roomID;
	std::atomic<int> next_id; // 임시용. 원래는 DB에 저장된 id를 입력해야 해서 이 부분이 불필요함.
	std::atomic<int> countClient;
	int maxClientsNum;

	std::unordered_map<int, SOCKETINFO*> client_map; // 현재 접속한 클라이언트 목록들
	CRITICAL_SECTION map_cs;

	std::vector<SOCKETINFO*> deletedClients;
	CRITICAL_SECTION deleteCS;
	*/

	// 게임 로직 부분
	ThreadSafeQueue<TaskQueueInput*> logicQueue;

public:
	GameRoom(int roomID): Room(roomID),
		logicQueue(INFINITE)
	{}

};



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
