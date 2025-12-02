#include "PacketIDExpand.h"
int PacketTypeGame::count = 0;

const int32_t PacketTypeGame::Hello = PacketTypeGame::Next();
const int32_t PacketTypeGame::Move = PacketTypeGame::Next();
const int32_t PacketTypeGame::FireBullet = PacketTypeGame::Next();
const int32_t PacketTypeGame::Die = PacketTypeGame::Next();
const int32_t PacketTypeGame::Resurrect = PacketTypeGame::Next();
