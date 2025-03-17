#pragma once

#include <crtdbg.h>

int RunServer(void);
int CheckMemoryLeaks(const _CrtMemState diff, const _CrtMemState end, bool returnVal);