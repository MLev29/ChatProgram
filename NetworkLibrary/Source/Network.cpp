#include "Network.h"
#include "ErrorManagement.h"

#include <iostream>

NetLib::Network::Network(void)
{
	m_socket = INVALID_SOCKET;
}

NetLib::Network::~Network(void)
{
	if (m_socket != INVALID_SOCKET)
	{
		int result = closesocket(m_socket);

		if (result == SOCKET_ERROR)
			NetLib::ReportWindowsError("Close socket", WSAGetLastError());
	}

	if (WSACleanup() == SOCKET_ERROR)
		NetLib::ReportWindowsError("WSACleanup", WSAGetLastError());
}

void NetLib::Network::InitWinSock(void)
{
	WSADATA data{};
	
	// Initialize WinSock2
	int result = WSAStartup(MAKEWORD(WINSOCK_MAJOR_VERSION, WINSOCK_MINOR_VERSION), &data);
	
	if (result)
	{
		NetLib::ReportWindowsError("WSAStartup", WSAGetLastError());
	
		std::exit(-1);
	}
	
	// Check version
	if (LOBYTE(data.wVersion) == WINSOCK_MAJOR_VERSION &&
		HIBYTE(data.wVersion) == WINSOCK_MINOR_VERSION)
		NetLib::ConsolePrint("WinSock2 successfully initialized\n");
	else
		NetLib::ConsolePrint("Incompatible version\n");
	
	SetCodePage();
}

void NetLib::Network::SetCodePage(void)
{
	SetConsoleCP(ANSI_LATIN);
	SetConsoleOutputCP(ANSI_LATIN);
}
