#include "Server.h"

#ifndef NDEBUG
// For memory leaks
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#ifdef NDEBUG // Release
int main(void)
{
	int returnValue = RunServer();

	return returnValue;
}
#else // Debug
int main(void)
{
	_CrtMemState start;
	_CrtMemCheckpoint(&start);

	int returnValue = RunServer();

	_CrtMemState end;
	_CrtMemCheckpoint(&end);

	_CrtMemState difference;

	if (_CrtMemDifference(&difference, &start, &end))
		CheckMemoryLeaks(difference, end, returnValue);

	return returnValue;
}
#endif

