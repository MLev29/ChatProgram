#include "Server.h"
#include "Network/Server.h"

#define WIN32_LEAN_AND_MEAN
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <Windows.h>

int RunServer(void)
{
	NetLib::Server server(DEFAULT_PORT);

	server.CreateSocket();

	server.Listen();

	while (server.IsRunning())
	{
		server.CheckSocketState();
	}

	return 0;
}

int CheckMemoryLeaks(const _CrtMemState diff, const _CrtMemState end, bool returnVal)
{
	(void) diff;
	(void) end;

	OutputDebugStringA("----- _CrtMemDumpStatistics -----");
	_CrtMemDumpStatistics(&diff);
	OutputDebugStringA("----- _CrtMemDumpStatistics -----");
	_CrtMemDumpAllObjectsSince(&end);
	OutputDebugStringA("----- _CrtDumpMemoryLeaks -----");
	_CrtDumpMemoryLeaks();
	OutputDebugStringA("----- _CrtCheckMemory -----");
	_CrtCheckMemory();

	if (!returnVal)
		return -1;

	return 0;
}