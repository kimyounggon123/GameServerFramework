#include "RoomManager.h"
RoomManager* RoomManager::instance = nullptr;


bool RoomManager::Initialize()
{
	if (isInitialized) return isInitialized;




	isInitialized = true;
	return isInitialized;
}
Room* RoomManager::GetRoom(int ID)
{
	if (ID == 0) return &allClients;
	return nullptr;
}



bool RoomManager::InputRoom(Room* room)
{
	try
	{
		if (room == nullptr) throw;

		
	}
	catch (...)
	{
		return false;
	}
	return true;
}
bool RoomManager::DeleteRoom(int ID)
{
	return true;
}
