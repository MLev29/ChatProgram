#include "UserInput.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <sstream>

std::string GetUserInput(const char* inputName)
{
	std::string userInput;

	// Get IP
	std::cout << "Enter " << inputName << ": ";
	std::getline(std::cin, userInput);
	std::cout << std::endl;

	if (userInput.empty())
	{
		std::cout << "Invalid " << inputName << " try again\n";

		return GetUserInput(inputName);
	}

	return userInput;
}

Message::Message(NetLib::Client* client)
{
	m_client = client;
}

std::string& Message::GetInputBuffer(void) noexcept
{
	return m_inputBuffer;
}

void Message::InputMessage(void)
{
	HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
	INPUT_RECORD inputRecord;
	DWORD eventCount;
	BOOL readResult;
	char newChar = ' ';

	readResult = ReadConsoleInputA(hConsole, &inputRecord, 1, &eventCount);
	if (readResult == 0)
		std::printf("ReadConsoleInput: %d\n", GetLastError());

	if (!inputRecord.Event.KeyEvent.bKeyDown)
		return;

	newChar = inputRecord.Event.KeyEvent.uChar.AsciiChar;

	switch (newChar)
	{
	case '\r':		// Enter
		Send();
		break;
	case '\b':		// Backspace
		DeleteCharacter();
		break;
	case '\x1B':	// Escape
		CloseClient();
		break;
	default:		// Any other characters
		AppendChar(newChar);
		break;
	}
}

void Message::AppendChar(char newChar)
{
	// Ignore following characters
	if (newChar == '\0' ||
		newChar == '\t')
		return;

	m_inputBuffer += newChar;

	std::printf("%c", newChar);
}

void Message::Send(void)
{
	if (m_inputBuffer.empty())
		return;

	std::stringstream sStream;
	sStream << m_client->GetUsername() << " > " << m_inputBuffer << '\n';

	m_client->SendData(sStream.str().c_str());
	m_inputBuffer.erase();
	std::printf("\n");
}

void Message::DeleteCharacter(void)
{
	if (m_inputBuffer.empty())
		return;

	HANDLE hOutConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

	GetConsoleScreenBufferInfo(hOutConsole, &consoleInfo);

	// Move cursor to end of previous line
	if (consoleInfo.dwCursorPosition.X == 0 &&
		consoleInfo.dwCursorPosition.Y != 0)
	{
		COORD prevLine
		{
			.X = consoleInfo.srWindow.Right,
			.Y = consoleInfo.dwCursorPosition.Y - 1
		};

		SetConsoleCursorPosition(hOutConsole, prevLine);
		std::printf(" ");
	}
	else
		std::printf("\b \b");

	m_inputBuffer.pop_back();
}

void Message::CloseClient(void)
{
	m_client->SetIsRunning(false);
}
