#include "DBconnector.h"


DBconnector::DBconnector(USHORT serverPort):
	exit_flag(false), sock(INVALID_SOCKET), addr{}, logs(Logs::getInstance()), 
	serverPort(serverPort),
	sessionManager(IOCPSessionManager::getInstance()), dispatcher(Dispatcher::getInstance()), gameDispatcher(GameDispatcher::getInstance()),
	hThreads{}, dwThreadID{}
{

}
DBconnector::~DBconnector()
{

	closesocket(sock);
	for (int i = 0; i < 2; i++)
	{
		CloseHandle(hThreads[i]);
	}
}


bool DBconnector::initialize()
{
	try {
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) throw "[DBconnector] socket()";

		//u_long mode = 1;
		//ioctlsocket(sock, FIONBIO, &mode); // 논블로킹 모드 활성화
		//WSAEventSelect(sock, hEvent, FD_READ | FD_WRITE | FD_CLOSE); // 읽기, 쓰기, 종료 이벤트

		// connect and bind socket
		// ZeroMemory로 구조체 초기화
		ZeroMemory(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(serverPort); // DB 서버 포트 번호로 설정

		// IP 주소를 sin_addr에 변환 및 저장
		if (inet_pton(AF_INET, DB_SERVER_IP, &(addr.sin_addr)) != 1) throw "[DBconnector] inet_pton() failed";

		INT retval = connect(sock, (SOCKADDR*)&addr, sizeof(addr));
		if (retval == SOCKET_ERROR) throw "connect()";
	}

	catch (const char* msg)
	{
		logs.log_error(msg, "DBconnector");
		logs.err_exit(_T("connect DB server"));
	}
	return true;
}

bool DBconnector::Start()
{
	try
	{
		// Make threads
		hThreads[0] = (HANDLE)_beginthreadex(
			NULL, 0,
			recvThread, this,
			0, (unsigned*)&dwThreadID[0]
		);
		if (hThreads[0] == NULL) throw "[DBconnector] _beginthreadex()";

		hThreads[1] = (HANDLE)_beginthreadex(
			NULL, 0,
			sendThread, this,
			0, (unsigned*)&dwThreadID[1]
		);
		if (hThreads[1] == NULL) throw "[DBconnector] _beginthreadex()";
	}
	catch (const char* msg)
	{
		logs.log_error(msg, "DBconnector");
		return false;
	}

	return true;
}
void DBconnector::Quit()
{
	exit_flag.store(false);

	// Recv 스레드가 recv()에서 탈출하도록
	shutdown(sock, SD_BOTH);
}



bool DBconnector::make_pk_and_push(char* recv_buf, int recv_len, size_t& offset) {

	TaskQueueInput* input = nullptr;
	bool result = true;
	int resyncCount = 0;

	try {
		while (recv_len - offset > 0) {
			if (!dispatcher.pop(input)) throw "Memory limit";
			if (input == nullptr) throw "input is nullptr!";

			ERROR_CODE code = input->packet->deserialize(recv_buf, recv_len, offset);
			if (code == ERROR_CODE::NEED_EXTRA_DATA)
			{
				dispatcher.push(input);
				break;
			}
			else if (code != ERROR_CODE::SUCCESS)
			{
				dispatcher.push(input);
				offset += 1; // 한 바이트씩 버리면서 다음 패킷 탐색
				resyncCount++;
				if (resyncCount >= 5)
				{
					// 너무 많이 재동기화 했으면 남은 데이터 모두 버림
					offset = recv_len;
					break;
				}
				continue;
			}

			// find who send this
			SOCKETINFO* whoSendPacket = nullptr;
			if (!sessionManager.find_socketinfo(input->packet->getClientID(), whoSendPacket)) throw "non-exist client";

			input->sessionInfo = whoSendPacket;
			if (!dispatcher.enqueue(input, QueueInformation::PacketProcess)) throw "enqueue()";
		}
	}

	catch (const char* msg) {
		if (input != nullptr) dispatcher.push(input);
		logs.log_error(msg, "DBconnector");
		result = false;
	}

	return result;
}
unsigned int DBconnector::recvThread(LPVOID lpParam)
{
	DBconnector* This = reinterpret_cast<DBconnector*>(lpParam);

	INT retval = 0;

	int recv_len = 0; // 현재까지 수신된 데이터
	size_t offset = 0; // 읽는 버퍼의 위치
	char recv_buf[BUFFERSIZE + 1];

	Logs& logs = This->logs;

	while (!This->exit_flag.load()) 
	{
		try {
			retval = recv(This->sock, recv_buf + recv_len, BUFFERSIZE - recv_len, 0);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK) throw "recv() 실패";
				return 0; // 데이터 없음 → 즉시 반환
			}

			if (retval == 0) throw (_T("서버가 연결을 종료했습니다."));

			recv_len += retval;  // 수신된 데이터 길이 증가

			bool result = This->make_pk_and_push(recv_buf, recv_len, offset);
			if (!result) throw "make_pk_and_push()";

			memmove(recv_buf, recv_buf + offset, recv_len - offset); // 남은 데이터 이동
			recv_len -= static_cast<int>(offset);
			offset = 0;
		}

		catch (const char* msg) {
			logs.log_error(msg, "DBconnector");
		}

	}
	return 0;
}
unsigned int DBconnector::sendThread(LPVOID lpParam)
{
	DBconnector* This = reinterpret_cast<DBconnector*>(lpParam);
	Dispatcher& dispatcher = This->dispatcher;
	Logs& logs = This->logs;
	GameDispatcher& gameDispatcher = This->gameDispatcher;

	INT retval = 0;
	char send_buf[BUFFERSIZE + 1];

	while (!This->exit_flag.load())
	{
		TaskQueueInput* output = nullptr;
		try {
			if (!gameDispatcher.PopDBTask(output)) continue;

			if (output == nullptr) throw "output error";
			if (output->isInvalid()) throw "output field error";

			// serialize
			output->packet->setClientID(output->sessionInfo->id); // 후에 클라이언트 조회를 위해 패킷에 클라이언트 id input
			ERROR_CODE code = output->packet->serialize(send_buf);
			if (code != ERROR_CODE::SUCCESS) throw "serialize";

			// send packet 
			retval = send(This->sock, send_buf, output->packet->getPacketSerializedLength(), 0);
			if (retval == SOCKET_ERROR) throw "send()";
		}
		catch (const char* msg) {
			logs.log_error(msg, "DBconnector");
		}

		if (output != nullptr)	dispatcher.push(output);
	}
	return 0;
}