#include "Utils/stdafx.h"
#include "PacketGameProcess.h"
#include "ServerFramework.h"
int main()
{

	PacketGameProcess* gameProc = new PacketGameProcess();
	gameProc->initialize();

	
	ServerFramework* framework = new ServerFramework(std::move(gameProc), 1000, 1001, true, true);
	framework->Initialize();
	//framework->TurnOnDBconnector();

	framework->Run();
	SAFE_FREE(framework);
	

	/*
	Packet pk(0, PacketType::ServerIsClosed, PacketResult::Success);
	size_t offset = 0;
	pk.inputString("hello world!!!!", offset);
	pk.inputDataInt(2, offset);
	pk.inputDataFloat(3.24f, offset);

	char buffer[1042];
	int size = 0;
	ERROR_CODE err = pk.serialize(buffer, size);
	pk.print_packet_contents("WQEw");

	Packet temp;
	offset = 0;
	std::string hello;
	temp.deserialize(buffer, size, offset);

	offset = 0;
	int qwewqe;
	float test;
	temp.copyString(hello, offset);
	temp.copyDataInt(&qwewqe, offset);
	temp.copyDataFloat(&test, offset);
	temp.print_packet_contents("WQEw");
	std::cout << hello << qwewqe << test<< std::endl;
	*/

	return 0;
}