#ifndef _BROADCASTMANAGER_H
#define _BROADCASTMANAGER_H

#include "stdafx.h"
#include "ThreadPool.h"
#include "PacketGameProcess.h"

/// <summary>
/// >> 게임 서버용 확장 서버 프레임워크
/// 기존의 서버 프레임워크는 1:1 통신만 원할하기에
/// 확장을 통해 1:N 통신도 가능하게 확장하는 걸 증명하기 위한 서버 코드
/// 확장해야 할 부분은 BroadcastManager 추가 및 ServerFramework 상속 및 확장.
/// 
/// 필요한 부분:
/// Dispatcher가 아닌 별도의 pool을 추가하는 것이 좋아보임.

/// PacketProcess (플래그 + 대상 Room/세션 설정)
///      │
///      └─> BroadcastTaskPool (별도)
///              │
///              └─> BroadcastManager (별도 스레드)
///                       └─> 대상 세션 반복 WSASend
/// </summary>
/// 
/// 


class BroadcastManager : public ThreadPool
{
	IOCPSessionManager& sessionManager;
	DispatcherHub& dispatcherHub;

	SOCKET sockUDP;

	unsigned int workLoop() override;
	bool SendAllRoomMember(const Task& info);

public:
	BroadcastManager(int poolCapacity, SOCKET sockUDP)
		: ThreadPool(poolCapacity), sockUDP(sockUDP),
		dispatcherHub(DispatcherHub::getInstance()),
		sessionManager(IOCPSessionManager::getInstance())
	{}
	~BroadcastManager()
	{
	}
	bool initialize() override;
};
#endif