#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#include <iostream>

#include "memory.h"

#pragma comment(lib, "psapi.lib")

void* FindPatternInModule(HMODULE hModule, BYTE* pattern, SIZE_T patternSize) {
	if (hModule == nullptr) {
		std::cout << "module is null????" << std::endl;
		return nullptr;
	}

	// Get the module information to determine its memory range
	MODULEINFO moduleInfo;
	if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO))) {
		std::cout << "could not get module information" << std::endl;
		return nullptr;
	}

	BYTE* startAddress = reinterpret_cast<BYTE*>(moduleInfo.lpBaseOfDll);
	SIZE_T searchSize = moduleInfo.SizeOfImage;

	for (SIZE_T i = 0; i <= searchSize - patternSize; ++i) {
		bool found = true;
		for (SIZE_T j = 0; j < patternSize; ++j) {
			if (pattern[j] != 0xCC && pattern[j] != startAddress[i + j]) {
				found = false;
				break;
			}
		}
		if (found) {
			return &startAddress[i];
		}
	}

	return nullptr;
}