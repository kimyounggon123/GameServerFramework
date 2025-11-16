#pragma once

#include "stdafx.h"
#include <Ws2tcpip.h>  // for inet_pton or InetPton
#include <Windows.h>
#include "SOCKETINFO.h"
#include "ThreadPool.h"

#include "Dispatcher.h"
#include "PacketGameProcess.h"
#define DB_SERVER_IP "127.0.0.1"


/// <summary>
/// Dispatehr
/// 
///
/// </summary>
class DBconnector
{
	std::atomic<bool> exit_flag;

	USHORT serverPort;
	SOCKET sock;
	SOCKADDR_IN addr;
	
	HANDLE hThreads[2];
	HANDLE dwThreadID[2];

	IOCPSessionManager& sessionManager;
	Logs& logs;

	Dispatcher& dispatcher;
	GameDispatcher& gameDispatcher;

	bool make_pk_and_push(char* recv_buf, int recv_len, size_t& offset);
public:
	DBconnector(USHORT serverPort);
	~DBconnector();

	bool initialize();
	bool Start();
	void Quit();

	static unsigned int recvThread(LPVOID lpParam);
	static unsigned int sendThread(LPVOID lpParam);
};

