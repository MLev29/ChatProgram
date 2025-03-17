#include "Client.h"
#include "UserInput.h"
#include "Network/Client.h"

#include <iostream>
#include <string>

#define WIN32_LEAN_AND_MEAN
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <Windows.h>

int RunClient(void)
{
	// Init client
	std::string ipAddress = GetUserInput("IP address");
	std::string port = GetUserInput("Port");
	std::string username = GetUserInput("username");

	NetLib::Client client(username.c_str(), ipAddress.c_str(), port.c_str());
	Message messageInput(&client);

	// Connect to server
	client.ConnectClient();

	NetLib::EState state = NetLib::EState::NONE;
	std::string userInput;

	// Update client
	while (client.IsRunning())
	{
		state = client.SocketState();

		if (state == NetLib::EState::SEND)
			messageInput.InputMessage();
		else if (state == NetLib::EState::RECEIVE)
			client.ReceiveData(messageInput.GetInputBuffer());

	}

	std::string stopConsoleClose;
	std::printf("\nClient disconnected, press enter to close client\n");
	std::getline(std::cin, stopConsoleClose);

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
