#ifndef _DISPATCHERHUB_H
#define _DISPATCHERHUB_H

#include "Dispatcher.h"

enum class Route
{
	PacketProcess,
	Send,
	DB,
	Broadcast
};


// 게임 서버용 추가 디스패처
// TaskQueueInput에 저장된 정보들을 해당 Dispatcher의 풀에서 꺼내와서 deep copy
class DispatcherHub
{
	Dispatcher& dispatcher;
	DispatcherUnit* DBDispatcher;
	DispatcherUnit* BroadcastDispatcher;

	static DispatcherHub* instance;
	DispatcherHub() : dispatcher(Dispatcher::getInstance()),
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

#endif