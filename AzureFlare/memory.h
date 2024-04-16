#pragma once
#include <windows.h>

void* FindPatternInModule(HMODULE hModule, BYTE* pattern, SIZE_T patternSize);