#pragma once

#include "Network.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace NetLib
{
	class Server final : public Network
	{
	public:
				Server(void) = delete;
				Server(short port);
				~Server(void);

		void	CreateSocket(void);
		void	Listen(void);
		void	AcceptClientConnection(void);
		void	SendMsg(const char* message, unsigned long long clientSocket);
		void	SendMsgToAll(const char* message, unsigned long long senderSocket);
		void	CheckSocketState(void);
		bool	IsRunning(void);
		std::string					ReceiveData(unsigned long long socket);
		std::vector<std::string>	GetAllUsers(void);
	private:
		void	AddSocket(unsigned long long socket);
		void	RemoveSocket(int const socketIndex);
		void	RemoveSocket(unsigned long long socket);
		void	DisplayServerInfo(void);
		void	JoinMessage(unsigned long long clientSocket, const char* clientIP);

		std::unordered_map<unsigned long long,	std::string>	m_clientName;
		std::string	allUsers;
		void*	m_pollfdVec;
		short	m_port;
		bool	m_isRunning;
	};
}