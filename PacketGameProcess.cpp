#include "PacketGameProcess.h"
DispatcherHub* DispatcherHub::instance = nullptr;
bool DispatcherHub::Initialize()
{
	DBDispatcher = new DispatcherUnit();
	BroadcastDispatcher = new DispatcherUnit();

	if (!DBDispatcher || !BroadcastDispatcher) return false;

	DBDispatcher->initialize();
	BroadcastDispatcher->initialize();

	return true;
}

bool DispatcherHub::EnqueueProcessCopy(TaskQueueInput*& input, const Route& route)
{
	if (input == nullptr) return false;

	TaskQueueInput* copyThis = nullptr;
	DispatcherUnit* where = nullptr;
	int32_t packetResult= 0;

	switch (route)
	{
	case Route::Broadcast:
		where = BroadcastDispatcher;
		packetResult = PacketResult::BroadCast;
		break;
	case Route::DB:
		where = DBDispatcher;
		packetResult = PacketResult::WaitDatabase;
		break;
	}
	if (where == nullptr) return false;


	if (!where->popPool(copyThis)) return false;
	copyThis->copyFrom(*input);
	copyThis->packet->set_process_result(packetResult);
	if (!where->enqueue(copyThis))
	{
		where->pushPool(copyThis);
		return false;
	}
	return true;
}

bool DispatcherHub::DequeueProcess(TaskQueueInput*& output, const Route& route)
{
	DispatcherUnit* where = nullptr;

	switch (route)
	{
	case Route::Broadcast:
		where = BroadcastDispatcher;
		break;
	case Route::DB:
		where = DBDispatcher;
		break;
	}
	if (where == nullptr) return false;

	return where->dequeue(output);
}

bool DispatcherHub::ReturnTask(TaskQueueInput*& returnThis, const Route& route)
{
	bool result = false;

	switch (route)
	{
	case Route::Broadcast:
		result = BroadcastDispatcher->pushPool(returnThis);
		break;

	case Route::DB:
		result = DBDispatcher->pushPool(returnThis);
		break;

	default:
		break;
	}

	return result;
}


bool PacketGameProcess::BroadCastThis(TaskQueueInput* input, int RoomID)
{
	try
	{
		if (!input || !input->packet) throw "input nullptr";
		input->target.room = roomManager.GetRoom(RoomID);
		if (!input->target.room) throw "room nullptr";
		if (!dispatcherHub.EnqueueProcessCopy(input, Route::Broadcast)) throw "InputBroadcastTask()";
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "BroadCastThis()");
		return false;
	}

	return true;
}

bool PacketGameProcess::CallDBagent(TaskQueueInput* input)
{
	try
	{
		if (!input || !input->packet) throw "input nullptr";
		if (!dispatcherHub.EnqueueProcessCopy(input, Route::DB)) throw "InputBroadcastTask()";
	}
	catch (const char* msg)
	{
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

		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
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
		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
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
		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
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
		BroadCastThis(input, 0); // 특정 작업을 타 클라이언트에게 broadcast (0: Global broadcasting)
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
		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
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
		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
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