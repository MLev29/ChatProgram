#include "ErrorManagement.h"

void NetLib::ConsolePrint(LPCSTR format, ...)
{
	// Handle arguments
	va_list args;
	va_start(args, format);

	LPCSTR buffer;
	DWORD charCount = FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER, format, 0, 0, (LPSTR) & buffer, 0, &args);

	// Error handling (FormatMessage can fail)
	if (charCount)
	{
		// Get console handle
		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

		// Print message
		BOOL success = WriteConsoleA(out, buffer, charCount, nullptr, NULL);

		// if 0 write console failed
		if (success == 0)
		{
			// Write debug message to output
			OutputDebugStringA("WriteConsole: error\n");
		}
	}

	va_end(args);
}

void NetLib::ReportWindowsError(LPCSTR context, DWORD errorCode)
{
	/*
	*	Format message docs:
	*	https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessage
	*/

	LPCSTR buffer;

	DWORD charCount = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorCode, 0, (LPSTR) &buffer, 0, nullptr);

	if (charCount)
		ConsolePrint("%1: %2", context, buffer);
	else
		ConsolePrint("%1: error: %2\nFormatMessage: error %3", context, errorCode, GetLastError());
}