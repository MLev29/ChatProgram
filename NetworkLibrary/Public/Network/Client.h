#pragma once
#include "Network.h"

#include <string>

namespace NetLib
{
	enum EState
	{
		SEND,
		RECEIVE,
		NONE
	};

	class Client final : public Network
	{
	public:
					Client(void) = delete;
					Client(const char* username, const char* ipAddress, const char* port);

					~Client(void);

		std::string	GetUsername(void) const noexcept;
		bool		IsRunning(void) const noexcept;
		void		SetIsRunning(bool value);

		void		ConnectClient(void);
		void		SendData(const char* data);
		void		ReceiveData(std::string const& userInput);
		EState		SocketState(void);

	private:
		void		CreateSocket(int afFamily);

		const char*	m_ip;
		const char*	m_username;
		const char*	m_port;
		bool		m_isRunning;
	};
}