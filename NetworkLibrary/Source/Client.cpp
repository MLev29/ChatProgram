
#include "Client.h"
#include "ErrorManagement.h"

#define MAX_MESSAGE_LENGTH 1000
#define SERVER_DISCONNECT 10054

NetLib::Client::Client(const char* username, const char* ipAddress, const char* port)
{
	m_ip = ipAddress;
	m_port = port;
	m_username = username;
	m_isRunning = true;

	SetConsoleTitleA("Client");

	InitWinSock();
}

NetLib::Client::~Client()
{
	if (m_socket != INVALID_SOCKET)
	{
		if (shutdown(m_socket, SD_BOTH) == SOCKET_ERROR)
			NetLib::ReportWindowsError("Shutdown", GetLastError());
	}
}

void NetLib::Client::CreateSocket(int afFamily)
{
	// Create TCP socket of either IPv4 or IPv6 family
	m_socket = socket(afFamily, SOCK_STREAM, IPPROTO_TCP);
	
	// Error check
	if (m_socket == INVALID_SOCKET)
		NetLib::ReportWindowsError("Socket", GetLastError());
}

void NetLib::Client::ConnectClient()
{
	int result = 0;
	
	// Socket option error handling
	if (result == SOCKET_ERROR)
	{
		NetLib::ReportWindowsError("Setsockopt", GetLastError());
		m_isRunning = false;
	}

	addrinfo* resultInfo = nullptr;
	addrinfo socketAddress{};
	socketAddress.ai_family = AF_UNSPEC;
	socketAddress.ai_socktype = SOCK_STREAM;

	result = getaddrinfo(m_ip, m_port, &socketAddress, &resultInfo);
	
	if (result)
	{
		NetLib::ReportWindowsError("GetAddrInfo", GetLastError());
		m_isRunning = false;
		return;
	}

	// Create socket as we now know the family (IPv4 or IPv6)
	CreateSocket(resultInfo->ai_family);

	/*
	*	Connect to server, use macro INET6_ADDRSTRLEN in order to have enough space
	*	to use either an IPv4 or IPv6 address
	*/
	result = connect(m_socket, resultInfo->ai_addr, INET6_ADDRSTRLEN);

	if (result == SOCKET_ERROR)
	{
		NetLib::ReportWindowsError("Connect", GetLastError());
		m_isRunning = false;
	}

	// Send username data to server
	SendData(m_username);
}

void NetLib::Client::SendData(const char* data)
{
	int dataLength = (int) std::strlen(data);

	/*
	*	Limit message length in order to avoid message size being above
	*	maximum packet size
	*/
	if (dataLength > MAX_MESSAGE_LENGTH)
	{
		std::printf("\nFailed to send message, cannot send message longer than %d characters\n", MAX_MESSAGE_LENGTH);
		return;
	}
	
	// Send data to server
	int result = send(m_socket, data, dataLength, 0);
	
	if (result == SOCKET_ERROR)
	{
		DWORD error = GetLastError();

		// Server disconnect
		if (error == SERVER_DISCONNECT)
		{
			NetLib::ConsolePrint("\nServer not responding, unable to send message.\n");
			return;
		}

		NetLib::ReportWindowsError("Send", GetLastError());
	}
}

void NetLib::Client::ReceiveData(std::string const& userInput)
{
	HANDLE hOutConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	char buffer[MAX_MESSAGE_LENGTH];
	ZeroMemory(buffer, MAX_MESSAGE_LENGTH);

	int result = recv(m_socket, buffer, MAX_MESSAGE_LENGTH, 0);

	if (result == SOCKET_ERROR)
		NetLib::ReportWindowsError("Receive", GetLastError());
	else
	{
		// Check if user input buffer is empty
		if (!userInput.empty())
		{
			// Remove input
			for (int i = 0; i < userInput.length(); ++i)
			{
				GetConsoleScreenBufferInfo(hOutConsole, &consoleInfo);

				// Move cursor to previous line
				if (consoleInfo.dwCursorPosition.X == 0 &&
					consoleInfo.dwCursorPosition.Y != 0)
				{
					COORD prevLine
					{
						.X = consoleInfo.srWindow.Right,
						.Y = consoleInfo.dwCursorPosition.Y - 1
					};

					SetConsoleCursorPosition(hOutConsole, prevLine);
					NetLib::ConsolePrint(" ");
				}
				else
					NetLib::ConsolePrint("\b \b");
			}
		}

		NetLib::ConsolePrint("%1%2", buffer, userInput.c_str());
	}
}

NetLib::EState NetLib::Client::SocketState(void)
{
	// Create event
	HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);	// Input console
	HANDLE hSocket = WSACreateEvent();					// Receive (output)
	HANDLE hArray[2] = {hConsole, hSocket};
	
	int selectResult = WSAEventSelect(m_socket, hSocket, FD_READ);
	
	if (selectResult == SOCKET_ERROR)
		NetLib::ReportWindowsError("WSAEventSelect", GetLastError());

	// Wait for next event
	DWORD result = WaitForMultipleObjects(ARRAYSIZE(hArray), hArray, FALSE, INFINITE);
	
	if (result == WAIT_FAILED)
		NetLib::ReportWindowsError("WaitForMultipleObjects", GetLastError());

	NetLib::EState state = EState::NONE;
	switch (result)
	{
	case WAIT_OBJECT_0:
		state = EState::SEND;
		break;
	case WAIT_OBJECT_0 + 1:
		state = EState::RECEIVE;
		break;
	default:
		break;
	}

	// Return current event
	return state;
}

bool NetLib::Client::IsRunning(void) const noexcept
{
	return m_isRunning;
}

void NetLib::Client::SetIsRunning(bool value)
{
	m_isRunning = value;
}

std::string NetLib::Client::GetUsername(void) const noexcept
{
	return std::string(m_username);
}
