#pragma once

#include "Network/Client.h"
#include <string>

std::string GetUserInput(const char* inputName);

class Message
{
public:
	Message(NetLib::Client* client);
	~Message(void) = default;

	std::string& GetInputBuffer(void) noexcept;

	void InputMessage(void);

private:
	void AppendChar(char newChar);
	void Send(void);
	void DeleteCharacter(void);
	void CloseClient(void);

	std::string m_inputBuffer;
	NetLib::Client* m_client;
};