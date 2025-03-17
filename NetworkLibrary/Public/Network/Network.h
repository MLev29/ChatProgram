#pragma once

#define WINSOCK_MAJOR_VERSION 2
#define WINSOCK_MINOR_VERSION 2

#define DEFAULT_PORT 3983
#define ANSI_LATIN 1252

namespace NetLib
{
	class Network
	{
	public:
			Network(void);
			~Network(void);
	protected:
		void InitWinSock(void);
	
		unsigned long long m_socket;
	private:
		void SetCodePage(void);
	};
}