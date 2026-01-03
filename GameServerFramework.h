#ifndef _GAMESERVERFRAMEWORK_H
#define _GAMESERVERFRAMEWORK_H


#include "ServerFramework.h"
#include "BroadcastManager.h"
#include "DBconnector.h"

#include "PacketGameProcess.h"
class GameServerFramework : public ServerFramework
{
	BroadcastManager* broadcaster;

	USHORT DBport;
	DBconnector* dbConnector;


	DispatcherHub& gameDispatcher;
	bool Start() override;
	void Quit() override;
public:
	GameServerFramework(PacketProcess* packetProc = nullptr,
		USHORT portTCP = 1000, USHORT portUDP = 1001, USHORT DBport = 1002):
		ServerFramework(packetProc, portTCP, portUDP),
		broadcaster(nullptr), DBport(DBport), dbConnector(nullptr),
		gameDispatcher(DispatcherHub::getInstance())
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