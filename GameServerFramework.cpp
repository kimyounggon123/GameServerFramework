#include "GameServerFramework.h"

bool GameServerFramework::initialize()
{
	if (!ServerFramework::initialize()) return false;
	try
	{
		broadcaster = new BroadcastManager(5);
		if (!broadcaster || !broadcaster->initialize()) throw "broadcaster";
	}
	catch (const char* msg)
	{
		logs.log(msg, "GameServerFramework::initialize()");
		return false;
	}

	return true;
}

bool GameServerFramework::TurnOnDBconnector()
{
	dbConnector = new DBconnector(dbPort);
	if (!dbConnector || !dbConnector->initialize()) return false;
	return true;
}

bool GameServerFramework::Start()
{
	if (!ServerFramework::Start()) return false;

	try
	{
		if (broadcaster && !broadcaster->Start()) throw "broadcaster";
		if (dbConnector && !dbConnector->Start()) throw "dbConnector";
	}
	catch (const char* msg)
	{
		logs.log(msg, "GameServerFramework::Start()");
		return false;
	}
	return true;

}
void GameServerFramework::Quit()
{
	ServerFramework::Quit();
	if (broadcaster) broadcaster->Quit();
	if (dbConnector) dbConnector->Quit();
	gameDispatcher.UndoAllPool();
}
