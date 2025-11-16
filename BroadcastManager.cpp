#include "BroadcastManager.h"


bool BroadcastManager::initialize()
{
	if (!ThreadPool::initialize()) return false;
	gameDispatcher.Initialize();
	return true;
}

bool BroadcastManager::SendAllRoomMember(BroadcastInformation* info)
{
	std::vector<SOCKETINFO*> members;
	info->room->CopyMemberPointers(members);


	INT retval;
	DWORD sendbytes;

	char buffer[2048]; // 미리 버퍼에 serialize
	if (!info->packet->serialize(buffer)) return false; // 패킷이 망가져있을 경우 보내지 않기
	ULONG len = info->packet->getPacketSerializedLength();

	for (SOCKETINFO* ptr : members)
	{
		try
		{
			if (ptr == nullptr) throw "nullptr";

			memcpy(ptr->response.IO_buffer, buffer, len); // 미리 serialize한 버퍼를 복사
			ptr->response.reset_overlapped(ptr->response.IO_buffer, len); // wsabuf 초기화

			DWORD waitResult = ptr->waitSendEvent();
			if (waitResult != WAIT_OBJECT_0)
			{
				if (waitResult == WAIT_TIMEOUT)	throw "waitMutex() time up";
				if (waitResult == WAIT_FAILED) throw "waitSendEvent() failed";
			}

			retval = WSASend(ptr->sock, &ptr->response.wsabuf, 1, &sendbytes,
				0, &ptr->response.overlapped, NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					throw "WSASend()";
				}
			}
			ptr->addResponseCount();
		}
		catch (const char* msg)
		{
			logs.log(msg, "SendAllRoomMember()");
		}
	}


	return true;
}

unsigned int BroadcastManager::workLoop()
{
	while (!exit_flag.load())
	{
		BroadcastInformation* output = nullptr;
		
		try
		{
			if (!gameDispatcher.PopBroadCastTask(output)) continue;
			if (output == nullptr) throw "output error";
			if (output->isInvalid()) throw "output field error";

			if (!SendAllRoomMember(output)) throw "SendAllRoomMember()";
		}
		catch (const char* msg)
		{
			logs.log_error(msg, "SendManager::work()");
		}

		if (output != nullptr)
		{
			gameDispatcher.PushTaskToBroadCastPool(output);
		}
	}
	return 0;
}