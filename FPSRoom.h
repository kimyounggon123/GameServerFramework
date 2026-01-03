#ifndef _FPSROOM_H
#define _FPSROOM_H

#include "SOCKETINFO.h"

#include "Frame.h"
class FPSRoom : public Room
{
	/*
	int roomID;
	std::atomic<int> next_id; // 임시용. 원래는 DB에 저장된 id를 입력해야 해서 이 부분이 불필요함.
	std::atomic<int> countClient;
	int maxClientsNum;

	std::unordered_map<int, SOCKETINFO*> client_map; // 현재 접속한 클라이언트 목록들
	CRITICAL_SECTION map_cs;

	std::vector<SOCKADDR_IN> udpTargets; // UDP 브로드캐스팅용
	CRITICAL_SECTION udpCS;

	std::vector<SOCKETINFO*>deletedClients;
	CRITICAL_SECTION deleteCS;
	*/


	float acc;
	FrameClass frame;

public:
	FPSRoom(int roomID, int maxClientNum = 8) : Room(roomID, maxClientNum),
		acc(0.0f),
		frame(TARGET_DT_144)
	{}

	void Frame();
	void Update() override;


};
#endif