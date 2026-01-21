#include "DispatcherHub.h"

DispatcherHub* DispatcherHub::instance = nullptr;
bool DispatcherHub::Initialize()
{
	DBDispatcher = new DispatcherUnit();
	BroadcastDispatcher = new DispatcherUnit();

	if (!DBDispatcher || !BroadcastDispatcher) return false;

	dispatcher.initialize();
	DBDispatcher->initialize();
	BroadcastDispatcher->initialize();

	return true;
}

bool DispatcherHub::EnqueueProcess(TaskPTR input, const Route& route)
{
	if (input == nullptr) return false;

	DispatcherUnit* where = nullptr;
	int32_t packetResult = 0;

	switch (route)
	{
	case Route::Broadcast:
		where = BroadcastDispatcher;
		packetResult = PacketResult::BroadCast;
		break;
	case Route::DB:
		where = DBDispatcher;
		packetResult = PacketResult::WaitDatabase;
		break;

	case Route::PacketProcess:
		dispatcher.enqueue(std::move(input), TaskInformation::PacketProcess);
		return true;
	case Route::Send:
		dispatcher.enqueue(std::move(input), TaskInformation::Send);
		return true;
	}

	if (where == nullptr) return true;


	input->packet->set_process_result(packetResult);
	if (!where->enqueue(std::move(input)))
	{
		return false;
	}

	return true;
}

bool DispatcherHub::DequeueProcess(TaskPTR& output, const Route& route)
{
	DispatcherUnit* where = nullptr;

	switch (route)
	{
	case Route::Broadcast:
		where = BroadcastDispatcher;
		break;
	case Route::DB:
		where = DBDispatcher;
		break;

	case Route::PacketProcess:
		dispatcher.dequeue(output, TaskInformation::PacketProcess);
		return true;
	case Route::Send:
		dispatcher.dequeue(output, TaskInformation::Send);
		return true;
	}
	if (where == nullptr) return false;

	return where->dequeue(output);
}

bool DispatcherHub::PushTask(TaskPTR returnThis, const Route& route)
{
	bool result = false;

	switch (route)
	{
	case Route::Broadcast:
		result = BroadcastDispatcher->pushPool(std::move(returnThis));
		break;

	case Route::DB:
		result = DBDispatcher->pushPool(std::move(returnThis));
		break;

	case Route::PacketProcess:
		dispatcher.push(std::move(returnThis), TaskInformation::PacketProcess);
		break;
	case Route::Send:
		dispatcher.push(std::move(returnThis), TaskInformation::Send);
		break;


	default:
		break;
	}

	return result;
}
bool DispatcherHub::PopTask(TaskPTR& returnThis, const Route& route)
{
	bool result = false;

	switch (route)
	{
	case Route::Broadcast:
		result = BroadcastDispatcher->popPool(returnThis);
		break;

	case Route::DB:
		result = DBDispatcher->popPool(returnThis);
		break;


	case Route::PacketProcess:
		dispatcher.pop(returnThis, TaskInformation::PacketProcess);
		break;
	case Route::Send:
		dispatcher.pop(returnThis, TaskInformation::Send);
		break;

	default:
		break;
	}
	return result;
}
