#ifndef _GAMESERVERFRAMEWORK_H
#define _GAMESERVERFRAMEWORK_H


#include "ServerFramework.h"
#include "BroadcastManager.h"
#include "DBconnector.h"

#include "PacketGameProcess.h"
class GameServerFramework : public ServerFramework
{
	BroadcastManager* broadcaster;

	USHORT dbPort;
	DBconnector* dbConnector;


	GameDispatcher& gameDispatcher;
	bool Start() override;
	void Quit() override;
public:
	GameServerFramework(PacketProcess* packetProc = nullptr, USHORT serverPort = 1000, USHORT dbPort = 1001):
		ServerFramework(packetProc, serverPort), broadcaster(nullptr), dbPort(dbPort), dbConnector(nullptr),
		gameDispatcher(GameDispatcher::getInstance())
	{ }
	~GameServerFramework()
	{
		SAFE_FREE(broadcaster);
		SAFE_FREE(dbConnector);
	}

	bool initialize() override;
	bool TurnOnDBconnector(); // DB 커넥션은 미리 사용할 지 설정하는 걸로 작성함.
};

#endif