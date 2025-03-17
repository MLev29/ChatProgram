#include "Server.h"
#include "ErrorManagement.h"

#include <sstream>

#define CLIENT_DISCONNECT 10053
#define CLIENT_DISCONNECT_ERROR 10054

NetLib::Server::Server(short port)
{
	m_port = port;
	m_isRunning = true;
	m_pollfdVec = new std::vector<pollfd>;

	// Set console title
	SetConsoleTitleA("Server");

	InitWinSock();
}

NetLib::Server::~Server(void)
{
	std::vector<pollfd>& pollfdVec = *static_cast<std::vector<pollfd>*>(m_pollfdVec);
	
	pollfdVec.~vector();
	delete m_pollfdVec;

	m_clientName.clear();
}

void NetLib::Server::CreateSocket(void)
{
	// Set address & port
	sockaddr_in6 address{};
	address.sin6_family = AF_INET6;
	address.sin6_port = htons(m_port);
	address.sin6_addr = in6addr_any;
	
	// Create socket
	m_socket = socket(address.sin6_family, SOCK_STREAM, IPPROTO_TCP);
	
	// Socket error handling
	if (m_socket == SOCKET_ERROR)
	{
		NetLib::ReportWindowsError("socket", WSAGetLastError());
		m_isRunning = false;
	}
	
	// Set socket options (for dual stack socket)
	int isIPV6only = 0;
	int result = setsockopt(m_socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&isIPV6only), sizeof(isIPV6only));
	
	// Socket option error handling
	if (result == SOCKET_ERROR)
	{
		NetLib::ReportWindowsError("setsockopt", GetLastError());
		m_isRunning = false;
	}
	
	// Bind socket
	result = bind(m_socket, (sockaddr*) &address, sizeof(address));
	
	// Bind error handling
	if (result == SOCKET_ERROR)
	{
		NetLib::ReportWindowsError("Bind", GetLastError());
		m_isRunning = false;
	}
	
	// Add socket to fd (file descriptor) array
	AddSocket(m_socket);
	
	// Display server socket info (IP & port)
	DisplayServerInfo();
}

void NetLib::Server::Listen(void)
{
	/*
	*	Listen for client connections.
	*	Use macro SOMAXCONN to allow the maximum amount of clients to connect
	*/
	int result = listen(m_socket, SOMAXCONN);

	if (result == SOCKET_ERROR)
		NetLib::ReportWindowsError("Listen", GetLastError());
}

void NetLib::Server::AcceptClientConnection(void)
{
	// Accept
	SOCKET clientSocket = NULL;
	sockaddr_storage socketStorage{};
	int socketStorageSize = sizeof(socketStorage);

	// Accept client connection
	clientSocket = accept(m_socket, (SOCKADDR*) &socketStorage, &socketStorageSize);

	if (clientSocket == INVALID_SOCKET)
	{
		NetLib::ReportWindowsError("Accept", GetLastError());
		m_isRunning = false;
	}
	
	LPCSTR clientIP[INET6_ADDRSTRLEN]{};
	DWORD clientIPSize = ARRAYSIZE(clientIP);

	// Get client IP
	if (WSAAddressToStringA((SOCKADDR*) &socketStorage, socketStorageSize, nullptr, (LPSTR) &clientIP, &clientIPSize))
		NetLib::ReportWindowsError("WSAAddressToStringA", GetLastError());
	
	// Append client data to FD array
	AddSocket(clientSocket);
	
	// Get client username
	std::string username = ReceiveData(clientSocket).c_str();
	m_clientName[clientSocket] = username.c_str();
	
	JoinMessage(clientSocket, (const char*) clientIP);
}

std::string NetLib::Server::ReceiveData(unsigned long long socket)
{
	constexpr int maxBufferLength = 1024;
	char buffer[maxBufferLength];
	ZeroMemory(buffer, maxBufferLength);

	int result = recv(socket, buffer, maxBufferLength, 0);

	if (result == SOCKET_ERROR)
		NetLib::ReportWindowsError("Receive", GetLastError());

	return std::string(buffer);
}

std::vector<std::string> NetLib::Server::GetAllUsers(void)
{
	std::vector<std::string> usernames;
	std::vector<pollfd>& pollfdVec = *static_cast<std::vector<pollfd>*>(m_pollfdVec);

	for (pollfd fd : pollfdVec)
	{
		if (fd.fd == m_socket)
			continue;
		
		std::string username = m_clientName.find(fd.fd)->second;

		usernames.push_back(username);
	}

	return usernames;
}

void NetLib::Server::SendMsg(const char* message, unsigned long long socket)
{
	int messageLength = (int) std::strlen(message);

	int result = send(socket, message, messageLength, 0);

	if (result == SOCKET_ERROR)
	{
		const DWORD error = GetLastError();

		// Client has disconnected
		if (error == CLIENT_DISCONNECT_ERROR ||
			error == CLIENT_DISCONNECT)
		{
			RemoveSocket(socket);

			return;
		}
		
		NetLib::ReportWindowsError("Send", error);
	}
}

void NetLib::Server::SendMsgToAll(const char* message, unsigned long long senderSocket)
{
	// Get all sockets
	std::vector<pollfd>& pollfdVec = *static_cast<std::vector<pollfd>*>(m_pollfdVec);

	for (pollfd fd : pollfdVec)
	{
		// Ignore client who sent message & listening socket
		if (fd.fd == senderSocket ||
			fd.fd == m_socket)
			continue;

		SendMsg(message, fd.fd);
	}
}

void NetLib::Server::CheckSocketState()
{
	std::vector<pollfd>& pollfdVec = *static_cast<std::vector<pollfd>*>(m_pollfdVec);
	int result = WSAPoll(pollfdVec.data(), (int) pollfdVec.size(), -1);
	
	if (result == SOCKET_ERROR)
	{
		NetLib::ReportWindowsError("Poll", GetLastError());
	}
	
	std::string recvBuffer;
	
	for (int i = 0; i < (int) pollfdVec.size(); ++i)
	{
		if (pollfdVec[i].revents & POLLHUP)
		{
			RemoveSocket(i);
			continue;
		}

		if (pollfdVec[i].revents & POLLIN)
		{
			if (pollfdVec[i].fd == m_socket)
			{
				AcceptClientConnection();
				continue;
			}
	
			recvBuffer = ReceiveData(pollfdVec[i].fd);
			NetLib::ConsolePrint("%1", recvBuffer.c_str());

			if (recvBuffer.find("!closeServer") != std::string::npos)
			{
				m_isRunning = false;
				return;
			}
			
			for (int j = 0; j < (int) pollfdVec.size(); ++j)
			{
				/*
				*	Skip if the socket is the same as the socket which sent the data
				*	or if socket is the server socket to prevent client receiving duplicate
				*	messages.
				*/
				if (pollfdVec[j].fd == pollfdVec[i].fd || pollfdVec[j].fd == m_socket)
					continue;
				
				SendMsg(recvBuffer.c_str(), pollfdVec[j].fd);
			}
		}
	}
}

bool NetLib::Server::IsRunning(void)
{
	return m_isRunning;
}

void NetLib::Server::AddSocket(unsigned long long socket)
{
	std::vector<pollfd>& pollfdVec = *static_cast<std::vector<pollfd>*>(m_pollfdVec);

	pollfd newfd
	{
		.fd = socket,
		.events = POLLIN,
		.revents = 0
	};

	pollfdVec.push_back(newfd);
}

void NetLib::Server::RemoveSocket(int const socketIndex)
{
	int result = 0;
	std::vector<pollfd>& pollfdVec = *static_cast<std::vector<pollfd>*>(m_pollfdVec);

	// Remove username
	if (pollfdVec[socketIndex].fd != m_socket)
	{
		m_clientName.erase(pollfdVec[socketIndex].fd);
	}

	result = shutdown(pollfdVec[socketIndex].fd, SD_BOTH);

	// Shutdown error handling
	if (result == SOCKET_ERROR)
		NetLib::ReportWindowsError("Shutdown", GetLastError());

	result = closesocket(pollfdVec[socketIndex].fd);

	// Close socket error handling
	if (result == SOCKET_ERROR)
		NetLib::ReportWindowsError("Close", GetLastError());

	// Remove from array
	pollfdVec.erase(pollfdVec.begin() + socketIndex);
}

void NetLib::Server::RemoveSocket(unsigned long long socket)
{
	std::vector<pollfd>& pollfdVec = *static_cast<std::vector<pollfd>*>(m_pollfdVec);

	// Remove from pollFD array
	for (int i = 0; i < (int) pollfdVec.size(); ++i)
	{
		if (pollfdVec[i].fd == socket)
		{
			RemoveSocket(i);
			
			return;
		}
	}

	NetLib::ConsolePrint("Remove socket error: socket not in array\n");
}

void NetLib::Server::DisplayServerInfo()
{
	char stringBuf[256];
	PADDRINFOA result;
	DWORD strLength = ARRAYSIZE(stringBuf);

	if (gethostname(stringBuf, strLength) == SOCKET_ERROR)
		NetLib::ReportWindowsError("gethostname", GetLastError());
	if (getaddrinfo(stringBuf, NULL, NULL, &result))
		NetLib::ReportWindowsError("GetAddrInfo", GetLastError());

	// Get IP address
	for (PADDRINFOA i = result; i != nullptr; i = i->ai_next)
	{
		if (WSAAddressToStringA(i->ai_addr, (DWORD) i->ai_addrlen, NULL, stringBuf, &strLength))
			NetLib::ReportWindowsError("WSAAddressToString", GetLastError());

		NetLib::ConsolePrint("Server IP: %1\n", stringBuf);
	}

	// Display port
	NetLib::ConsolePrint("Server port: %1!hu!\n", m_port);

	freeaddrinfo(result);
}

void NetLib::Server::JoinMessage(unsigned long long clientSocket, const char* clientIP)
{
	const char* username = m_clientName.find(clientSocket)->second.c_str();

	// Print message on server console with new user & its IP
	NetLib::ConsolePrint("User %1 connected with ip %2\n", username, clientIP);

	// Message all users of new user
	std::stringstream sStream;
	sStream << "User " << username << " has joined\n";

	// Display message 
	SendMsgToAll(sStream.str().c_str(), clientSocket);

	allUsers += "Current users: ";
	std::vector<std::string> userArray = GetAllUsers();
	for (int i = 0; i < userArray.size(); ++i)
	{
		allUsers += userArray[i];

		if (i != ((int) userArray.size() - 1))
			allUsers += ", ";
		else
			allUsers += '\n';
	}

	// Send message containing all current users to new user
	SendMsg(allUsers.c_str(), clientSocket);

	allUsers.clear();
}
