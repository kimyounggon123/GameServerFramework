#include "BroadcastManager.h"


bool BroadcastManager::initialize()
{
	if (!ThreadPool::initialize()) return false;
	gameDispatcher.Initialize();
	for (int i = 0; i < 100; i++)
	{
		SOCKETINFO* forBroadcast = new SOCKETINFO(SESSION_TYPE::UDP, true);
		sessionManager.InputSOCKETINFOforUDP(forBroadcast);
	}
	return true;
}

bool BroadcastManager::SendAllRoomMember(BroadcastInformation* info)
{
	if (info == nullptr) return false;

	INT retval;
	DWORD sendbytes;
	char buffer[2048]; // 미리 버퍼에 serialize
	if (!info->packet->serialize(buffer)) return false; // 패킷이 망가져있을 경우 보내지 않기
	ULONG len = info->packet->getPacketSerializedLength();

	SOCKETINFO* forUDPconnection = nullptr;
	try
	{
		if (info->sessionType == SESSION_TYPE::TCP)
		{
			std::vector<SOCKETINFO*> members;
			info->room->CopySOCKETINFOPointers(members);

			for (SOCKETINFO* ptr : members)
			{
				if (ptr == nullptr) throw "nullptr";

				memcpy(ptr->response.IO_buffer, buffer, len); // 미리 serialize한 버퍼를 복사
				ptr->response.reset_overlapped(ptr->response.IO_buffer, len); // wsabuf 초기화

				DWORD waitResult = ptr->waitSendEvent();
				if (waitResult != WAIT_OBJECT_0)
				{
					if (waitResult == WAIT_TIMEOUT)	throw "waitMutex() time up TCP";
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
		}

		else if (info->sessionType == SESSION_TYPE::UDP)
		{
			std::vector<SOCKADDR_IN> udpmember;
			info->room->CopyMemberPointersUDP(udpmember);

			for (SOCKADDR_IN udp : udpmember)
			{
				forUDPconnection = nullptr;
				if (!sessionManager.PopSOCKETINFOforUDP(forUDPconnection)) throw "pop fail!";

				/*DWORD waitResult = forUDPconnection->waitSendEvent();
				if (waitResult != WAIT_OBJECT_0)
				{
					if (waitResult == WAIT_TIMEOUT)	throw "waitMutex() time up";
					if (waitResult == WAIT_FAILED) throw "waitSendEvent() failed";
				}
				*/

				memcpy(forUDPconnection->response.IO_buffer, buffer, len); // 미리 serialize한 버퍼를 복사
				forUDPconnection->response.reset_overlapped(forUDPconnection->response.IO_buffer, len); // wsabuf 초기화

				// Sending data
				retval = WSASendTo(sockUDP,
					&forUDPconnection->response.wsabuf,
					1,
					&sendbytes,
					0,
					(SOCKADDR*)&udp,
					sizeof(SOCKADDR_IN),
					&forUDPconnection->response.overlapped,
					NULL);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						throw "WSASendTo()";
					}
				}
			}
		}
		else
		{
			return false;
		}
	}
	catch (const char* msg)
	{
		if (forUDPconnection) sessionManager.InputSOCKETINFOforUDP(forUDPconnection);
		logs.log(msg, "SendAllRoomMember()");
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