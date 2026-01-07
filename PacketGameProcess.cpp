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

bool DispatcherHub::EnqueueProcess(TaskPTR input, const Route& route)
{
	if (input == nullptr) return false;

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


	input->packet->set_process_result(packetResult);
	if (!where->enqueue(std::move(input)))
	{
		where->pushPool(std::move(input));
		return false;
	}

	return true;
}

bool DispatcherHub::DequeueProcess(TaskPTR& output, const Route& route)
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

bool DispatcherHub::PushTask(TaskPTR returnThis, const Route& route)
{
	bool result = false;

	switch (route)
	{
	case Route::Broadcast:
		result = BroadcastDispatcher->pushPool(std::move(returnThis));
		break;

	case Route::DB:
		result = DBDispatcher->pushPool(std::move(returnThis));
		break;

	default:
		break;
	}

	return result;
}
bool DispatcherHub::PopTask(TaskPTR& returnThis, const Route& route)
{
	bool result = false;

	switch (route)
	{
	case Route::Broadcast:
		result = BroadcastDispatcher->popPool(returnThis);
		break;

	case Route::DB:
		result = DBDispatcher->popPool(returnThis);
		break;

	default:
		break;
	}
	return result;
}

bool PacketGameProcess::BroadCastThis(Task& input, int RoomID)
{
	TaskPTR broadcast = nullptr;
	try
	{
		if (!dispatcherHub.PopTask(broadcast, Route::Broadcast)) throw "pop fail";

		input.target.room = roomManager.GetRoom(RoomID);
		if (!input.target.room) throw "room nullptr";

		broadcast->copyFrom(input);
		//broadcast->packet->set_process_result(PacketResult::BroadCast);

		if (!dispatcherHub.EnqueueProcess(std::move(broadcast), Route::Broadcast)) throw "InputBroadcastTask()";
	}
	catch (const char* msg)
	{
		if (broadcast) dispatcherHub.PushTask(std::move(broadcast), Route::Broadcast);
		logs.log_error(msg, "BroadCastThis()");
		return false;
	}

	return true;
}

bool PacketGameProcess::DBThis(Task& input)
{
	TaskPTR DBtask = nullptr;
	try
	{
		if (!dispatcherHub.PopTask(DBtask, Route::DB)) throw "pop fail";
		if (!input.target.room) throw "room nullptr";

		DBtask->copyFrom(input);
		//broadcast->packet->set_process_result(PacketResult::BroadCast);

		if (!dispatcherHub.EnqueueProcess(std::move(DBtask), Route::DB)) throw "InputBroadcastTask()";
	}
	catch (const char* msg)
	{
		if (DBtask) dispatcherHub.PushTask(std::move(DBtask), Route::DB);
		logs.log_error(msg, "DBThis()");
		return false;
	}

	return true;
}


void PacketGameProcess::initialize()
{
	PacketProcess::initialize();
	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Hello, PacketResult::Try },
		[this](Task& input) {return Hello(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Bye, PacketResult::Try },
		[this](Task& input) {return Bye(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Move, PacketResult::Try },
		[this](Task& input) {return Move(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::FireBullet, PacketResult::Try },
		[this](Task& input) {return FireBullet(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Die, PacketResult::Try },
		[this](Task& input) {return Die(input); }
	);

	func_map.emplace(
		PacketProcessKey{ PacketTypeGame::Resurrect, PacketResult::Try },
		[this](Task& input) {return Resurrect(input); }
	);

	isInitialized = true;
}

bool PacketGameProcess::Hello(Task& input)
{
	try
	{
		input.packet->setClientID(input.sessionInfo->id);

		std::vector<SOCKETINFO*> allClient;
		Room* room = roomManager.GetRoom(0);
		room->CopySOCKETINFOPointers(allClient);

		size_t offset = 0;
		for (SOCKETINFO* info : allClient)
		{
			if (!info->acceptCompleted || info->isBroadcast) continue;
			input.packet->inputDataInt(info->id, offset);
		}

		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
		input.packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "HelloClient()");
		input.packet->set_process_result(PacketResult::Fail);
		return false;
	}

	return true;
}

bool PacketGameProcess::Bye(Task& input)
{
	try
	{
		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
		input.packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "Bye");
		input.packet->set_process_result(PacketResult::Fail);
		return false;
	}
	return true;
}


bool PacketGameProcess::Move(Task& input)
{
	try
	{
		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
		input.packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "Move");
		input.packet->set_process_result(PacketResult::Fail);
		return false;
	}

	return true;
}

bool PacketGameProcess::FireBullet(Task& input)
{
	try
	{
		//gameDispatcher.InputDBTask(input); //작업을 DB에 저장하기 위해 전송함
		BroadCastThis(input, 0); // 특정 작업을 타 클라이언트에게 broadcast (0: Global broadcasting)
		input.packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "FireBullet");
		input.packet->set_process_result(PacketResult::Fail);
		return false;
	}

	return true;
}

bool PacketGameProcess::Die(Task& input)
{
	try
	{
		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
		input.packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "Die");
		input.packet->set_process_result(PacketResult::Fail);
		return false;
	}
	return true;
}

bool PacketGameProcess::Resurrect(Task& input)
{
	try
	{
		if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
		input.packet->set_process_result(PacketResult::Success);
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "Resurrect");
		input.packet->set_process_result(PacketResult::Fail);
		return false;
	}
	return true;
}