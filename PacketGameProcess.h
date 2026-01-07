#ifndef _PACKETGAMEPROCESS_H
#define _PACKETGAMEPROCESS_H

#include "PacketProcess.h"
#include "RoomManager.h"
#include "PacketIDExpand.h"

// 게임 서버용 추가 디스패처
// TaskQueueInput에 저장된 정보들을 해당 Dispatcher의 풀에서 꺼내와서 deep copy


enum class Route
{
	Process,
	Send,
	DB,
	Broadcast
};

class DispatcherHub
{
	DispatcherUnit* DBDispatcher;
	DispatcherUnit* BroadcastDispatcher;

	static DispatcherHub* instance;
	DispatcherHub():
		DBDispatcher(nullptr), BroadcastDispatcher(nullptr)
	{}
public:
	~DispatcherHub()
	{
		SAFE_FREE(DBDispatcher);
		SAFE_FREE(BroadcastDispatcher);
	}
	static DispatcherHub& getInstance()
	{
		if (instance == nullptr) instance = new DispatcherHub();
		return *instance;
	}

	bool Initialize();



	bool EnqueueProcess(TaskPTR input, const Route& route); // 1:1 통신 단위를 복사해서 전달해야 함
	bool DequeueProcess(TaskPTR& input, const Route& route);  // dispatcher에서 pop하는 작업. 복사 x

	bool PopTask(TaskPTR& returnThis, const Route& route);
	bool PushTask(TaskPTR returnThis, const Route& route);
};



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