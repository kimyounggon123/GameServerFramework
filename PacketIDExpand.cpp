#include "PacketIDExpand.h"
int PacketTypeGame::count = 0;
const int32_t PacketTypeGame::Hello = PacketTypeGame::Base + PacketTypeGame::Next();
const int32_t PacketTypeGame::FireBullet = PacketTypeGame::Base + PacketTypeGame::Next();
