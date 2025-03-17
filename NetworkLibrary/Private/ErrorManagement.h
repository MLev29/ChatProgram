#pragma once

// Prevent issues relating to unicode
#ifdef _UNICODE
#undef _UNICODE
#endif

#ifdef UNICODE
#undef UNICODE
#endif

// Enable unicode
#if 0
#define UNICODE
#define _UNICODE
#endif

// Macro to remove useless includes when including windows.h
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>

#ifdef UNICODE
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#endif

namespace NetLib
{
	void ConsolePrint(LPCSTR format, ...);
	void ReportWindowsError(LPCSTR context, DWORD errorCode);
}