#include "RoomManager.h"
RoomManager* RoomManager::instance = nullptr;


Room* RoomManager::GetRoom(int num)
{
	if (num == -1) return &allClients;
	return nullptr;
}