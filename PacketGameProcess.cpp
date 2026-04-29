#include "PacketGameProcess.h"

bool PacketGameProcess::BroadCastThis(Task& input, int RoomID)
{
	TaskPTR broadcastTask = nullptr;
	try
	{
		if (!dispatcherHub.BorrowTaskPTR(broadcastTask, DispatcherID_EX::Broadcast)) throw "pop fail";

		input.target.room = roomManager.GetRoom(RoomID);
		if (!input.target.room) throw "room nullptr";

		broadcastTask->copyFrom(input);
		broadcastTask->packet->set_process_result(PacketResult::BroadCast);

		if (!dispatcherHub.EnqueueTaskPTR(std::move(broadcastTask), DispatcherID_EX::Broadcast)) throw "InputBroadcastTask()";
	}
	catch (const char* msg)
	{
		/*
		if (broadcastTask != nullptr)
		{
			dispatcherHub.ReturnTaskPTR(std::move(broadcastTask), DispatcherID_EX::Broadcast);
		}
		*/

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
		if (!dispatcherHub.BorrowTaskPTR(DBtask, DispatcherID_EX::Database)) throw "pop fail";
		if (!input.target.room) throw "room nullptr";

		DBtask->copyFrom(input);
		DBtask->packet->set_process_result(PacketResult::WaitDatabase);

		if (!dispatcherHub.EnqueueTaskPTR(std::move(DBtask), DispatcherID_EX::Database)) throw "InputBroadcastTask()";
	}
	catch (const char* msg)
	{
		/*
		if (DBtask != nullptr)
		{
			dispatcherHub.ReturnTaskPTR(std::move(DBtask), DispatcherID_EX::Database);
		}
		*/
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