#include "RoomManager.h"
RoomManager* RoomManager::instance = nullptr;

bool GameRoom::EnqueueBroadcast(Packet*& input)
{
	return broadcastQueue.enqueue(input);
}
bool GameRoom::DequeueBroadcast(Packet*& output)
{
	return broadcastQueue.dequeue(output);
}


Room* RoomManager::GetRoom(int num)
{
	if (num == 0) return &allClients;
	return nullptr;
}