#ifndef _PACKETGAMEPROCESS_H
#define _PACKETGAMEPROCESS_H

#include "PacketProcess.h"
#include "RoomManager.h"
#include "PacketIDExpand.h"

struct BroadcastInformation
{
	Room* room;
	Packet* packet;
	SESSION_TYPE sessionType;
	BroadcastInformation(SESSION_TYPE s = SESSION_TYPE::TCP) :
		room(nullptr), packet(new Packet()), sessionType(s)
	{}
	~BroadcastInformation()
	{
		SAFE_FREE(packet);
	}

	bool isInvalid()
	{
		return room == nullptr || packet == nullptr;
	}
};

// 게임 서버용 추가 디스패처
// TaskQueueInput에 저장된 정보들을 해당 Dispatcher의 풀에서 꺼내와서 deep copy
class GameDispatcher
{
	bool isInitialized;
	ThreadSafeStack<TaskQueueInput*> DBpool;
	ThreadSafeStack<BroadcastInformation*> BroadCastpool;

	ThreadSafeQueue<TaskQueueInput*> DBagentTasks; // DBpool에서 pop한 후 복사한 후 여기에 push, 이후 전송할 때 DBpool에 push
	ThreadSafeQueue<BroadcastInformation*> BroadCastAgentTasks; // DBpool에서 pop한 후 복사한 후 여기에 push, 이후 전송할 때 DBpool에 push

	static GameDispatcher* instance;
	GameDispatcher() : isInitialized(false),
		DBpool(INFINITE), BroadCastpool(INFINITE),
		DBagentTasks(100), BroadCastAgentTasks(100)
	{}
public:
	~GameDispatcher()
	{
		UndoAllPool();
		while (!DBpool.isEmpty())
		{
			TaskQueueInput* delThis = nullptr;
			if (DBpool.pop(delThis))
				SAFE_FREE(delThis);
		}
		while (!BroadCastpool.isEmpty())
		{
			BroadcastInformation* delThis = nullptr;
			if (BroadCastpool.pop(delThis))
				SAFE_FREE(delThis);
		}
	}
	static GameDispatcher& getInstance()
	{
		if (instance == nullptr) instance = new GameDispatcher;
		return *instance;
	}
	void UndoAllPool();

	bool Initialize();

	bool InputDBTask(TaskQueueInput* src); // 원본 소스를 복사해서 DBagentTask에 전달
	bool PopDBTask(TaskQueueInput*& output);
	bool PushTaskToDBpool(TaskQueueInput*& src); // 사용한 Task 다시 return

	bool GetTaskFromBroadCastPool(BroadcastInformation*& getThis); // 미리 풀에서 받아오기
	bool InputBroadCastTask(BroadcastInformation*& src);  // 풀에서 받아온 정보를 task queue에 넣음
	bool PopBroadCastTask(BroadcastInformation*& output);
	bool PushTaskToBroadCastPool(BroadcastInformation*& src); // 전송 완료한 task를 다시 push
};




class PacketGameProcess : public PacketProcess
{
	GameDispatcher& gameDispatcher;
	RoomManager& roomManager;

	bool BroadCastThis(TaskQueueInput* input, int RoomID = 0, SESSION_TYPE type = SESSION_TYPE::TCP);

	bool HelloClient(TaskQueueInput* input);
	bool SomeoneFireBullet(TaskQueueInput* input);
	bool Move(TaskQueueInput* input);
public:
	PacketGameProcess(): PacketProcess(), gameDispatcher(GameDispatcher::getInstance()), roomManager(RoomManager::getInstance())
	{

	}
	void initialize() override;
};

#endif