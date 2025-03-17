#include "Client.h"

#ifndef NDEBUG
// For memory leaks
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#ifdef NDEBUG // Release
int main(void)
{
	int returnValue = RunClient();

	return returnValue;
}
#else // Debug
int main(void)
{
	_CrtMemState start;
	_CrtMemCheckpoint(&start);

	int returnValue = RunClient();

	_CrtMemState end;
	_CrtMemCheckpoint(&end);

	_CrtMemState difference;

	if (_CrtMemDifference(&difference, &start, &end))
		returnValue = CheckMemoryLeaks(difference, end, returnValue);

	return returnValue;
}
#endif

