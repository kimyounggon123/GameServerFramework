#include "PacketGameProcess.h"
GameDispatcher* GameDispatcher::instance = nullptr;

bool GameDispatcher::Initialize()
{
	if (isInitialized) return isInitialized;
	for (int i = 0; i < 100; i++)
	{
		BroadcastInformation* b = new BroadcastInformation();
		TaskQueueInput* task = new TaskQueueInput();

		BroadCastpool.push(b);
		DBpool.push(task);
	}

	isInitialized = true;
	return isInitialized;
}
void GameDispatcher::UndoAllPool()
{
	TaskQueueInput* output = nullptr;
	while (!DBagentTasks.isEmpty())
	{
		if (DBagentTasks.dequeue(output))
			DBpool.push(output);
	}

	BroadcastInformation* broadOut = nullptr;
	while (!BroadCastAgentTasks.isEmpty())
	{
		if (BroadCastAgentTasks.dequeue(broadOut))
			BroadCastpool.push(broadOut);
	}

}

bool GameDispatcher::InputDBTask(TaskQueueInput* src)
{
	TaskQueueInput* getFromPool = nullptr;
	if (!DBpool.pop(getFromPool)) return false;
	*getFromPool = *src; // deep copy

	if (!DBagentTasks.enqueue(getFromPool))
	{
		DBpool.push(getFromPool);
		return false;
	}

	return true;
}
bool GameDispatcher::PopDBTask(TaskQueueInput*& output)
{
	return DBagentTasks.dequeue(output);
}
bool GameDispatcher::PushTaskToDBpool(TaskQueueInput*& src)
{
	return DBpool.push(src);
}



// 미리 해당 클라이언트의 방 정보를 받아와야 함
bool GameDispatcher::GetTaskFromBroadCastPool(BroadcastInformation*& getThis)
{
	return BroadCastpool.pop(getThis);
}
bool GameDispatcher::InputBroadCastTask(BroadcastInformation*& src)
{
	return BroadCastAgentTasks.enqueue(src);
}
bool GameDispatcher::PopBroadCastTask(BroadcastInformation*& output)
{
	return BroadCastAgentTasks.dequeue(output);
}
bool GameDispatcher::PushTaskToBroadCastPool(BroadcastInformation*& src)
{
	return BroadCastpool.push(src);
}

bool PacketGameProcess::BroadCastThis(TaskQueueInput* input, int RoomID, SESSION_TYPE type)
{
	BroadcastInformation* forBroadcast = nullptr;

	try
	{
		if (!input || !input->packet) throw "input nullptr";

		if (!gameDispatcher.GetTaskFromBroadCastPool(forBroadcast)) throw "forBroadcast nullptr";

		forBroadcast->room = roomManager.GetRoom(RoomID);
		if (!forBroadcast->room) throw "room nullptr";

		forBroadcast->sessionType = type;
		*forBroadcast->packet = *input->packet; // deep copy


		forBroadcast->packet->set_process_result(PacketResult::BroadCast);
		if (!gameDispatcher.InputBroadCastTask(forBroadcast)) throw "InputBroadcastTask()";
	}
	catch (const char* msg)
	{
		if (forBroadcast) gameDispatcher.PushTaskToBroadCastPool(forBroadcast);
		logs.log_error(msg, "BroadCastThis()");
		return false;
	}

	return true;
}

void PacketGameProcess::initialize()
{
	PacketProcess::initialize();
	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Hello, PacketResult::Try },
		[this](TaskQueueInput* input) {return Hello(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Bye, PacketResult::Try },
		[this](TaskQueueInput* input) {return Bye(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Move, PacketResult::Try },
		[this](TaskQueueInput* input) {return Move(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::FireBullet, PacketResult::Try },
		[this](TaskQueueInput* input) {return FireBullet(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Die, PacketResult::Try },
		[this](TaskQueueInput* input) {return Die(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Resurrect, PacketResult::Try },
		[this](TaskQueueInput* input) {return Resurrect(input); }
	);

	isInitialized = true;
}



bool PacketGameProcess::Hello(TaskQueueInput* input)
{
	try
	{
		input->packet->setClientID(input->sessionInfo->id);

		std::vector<SOCKETINFO*> allClient;
		Room* room = roomManager.GetRoom(0);
		room->CopySOCKETINFOPointers(allClient);

		size_t offset = 0;
		for (SOCKETINFO* info : allClient)
		{
			if (!info->acceptCompleted || info->isBroadcast) continue;
			input->packet->inputDataInt(info->id, offset);
		}

		if (!BroadCastThis(input, 0, SESSION_TYPE::TCP)) throw "Broadcast Fail!";
		input->packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "HelloClient()");
		input->packet->set_process_result(PacketResult::Fail);
		return false;
	}

	return true;
}

bool PacketGameProcess::Bye(TaskQueueInput* input)
{
	try
	{
		if (!BroadCastThis(input, 0, SESSION_TYPE::TCP)) throw "Broadcast Fail!";
		input->packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "Bye");
		input->packet->set_process_result(PacketResult::Fail);
		return false;
	}
	return true;
}


bool PacketGameProcess::Move(TaskQueueInput* input)
{
	try
	{
		if (!BroadCastThis(input, 0, SESSION_TYPE::UDP)) throw "Broadcast Fail!";
		input->packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "Move");
		input->packet->set_process_result(PacketResult::Fail);
		return false;
	}

	return true;
}

bool PacketGameProcess::FireBullet(TaskQueueInput* input)
{
	try
	{
		//gameDispatcher.InputDBTask(input); //작업을 DB에 저장하기 위해 전송함
		BroadCastThis(input, 0, SESSION_TYPE::UDP); // 특정 작업을 타 클라이언트에게 broadcast (0: Global broadcasting)
		input->packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "FireBullet");
		input->packet->set_process_result(PacketResult::Fail);
		return false;
	}

	return true;
}

bool PacketGameProcess::Die(TaskQueueInput* input)
{
	try
	{
		if (!BroadCastThis(input, 0, SESSION_TYPE::UDP)) throw "Broadcast Fail!";
		input->packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "Die");
		input->packet->set_process_result(PacketResult::Fail);
		return false;
	}
	return true;
}

bool PacketGameProcess::Resurrect(TaskQueueInput* input)
{
	try
	{
		if (!BroadCastThis(input, 0, SESSION_TYPE::UDP)) throw "Broadcast Fail!";
		input->packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "Resurrect");
		input->packet->set_process_result(PacketResult::Fail);
		return false;
	}
	return true;
}