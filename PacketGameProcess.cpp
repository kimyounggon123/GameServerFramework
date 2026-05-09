#include "PacketGameProcess.h"




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
			if (!info->acceptCompleted || info->isDummy) continue;
			input.packet->inputDataInt(info->id, offset);
		}

		//if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";

		input.target.type = TARGET_TYPE::Room;
		input.target.room = room;
		input.broadcastFlag = true;
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
		//if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
		Room* room = roomManager.GetRoom(0);
		input.target.type = TARGET_TYPE::Room;
		input.target.room = room;
		input.broadcastFlag = true;

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
		//if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
		Room* room = roomManager.GetRoom(0);
		input.target.type = TARGET_TYPE::Room;
		input.target.room = room;
		input.broadcastFlag = true;

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
		//BroadCastThis(input, 0); // 특정 작업을 타 클라이언트에게 broadcast (0: Global broadcasting)

		Room* room = roomManager.GetRoom(0);
		input.target.type = TARGET_TYPE::Room;
		input.target.room = room;
		input.broadcastFlag = true;

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
		//if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";
		Room* room = roomManager.GetRoom(0);
		input.target.type = TARGET_TYPE::Room;
		input.target.room = room;
		input.broadcastFlag = true;

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
		//if (!BroadCastThis(input, 0)) throw "Broadcast Fail!";

		Room* room = roomManager.GetRoom(0);
		input.target.type = TARGET_TYPE::Room;
		input.target.room = room;
		input.broadcastFlag = true;

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

