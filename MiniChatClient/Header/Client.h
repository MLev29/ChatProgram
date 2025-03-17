#pragma once

#include <crtdbg.h>

int RunClient(void);
int CheckMemoryLeaks(const _CrtMemState diff, const _CrtMemState end, bool returnVal);