#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <iostream>
#include <cstdint>
#include <algorithm>

#include "memory.h"

// TODO: Get this based on the EXE, but these correspond to JPBB 1.25.11
#define PSOBB_CODE_REGION_START 0x401000
#define PSOBB_CODE_REGION_END PSOBB_CODE_REGION_START + 0x638000

#pragma comment(lib, "psapi.lib")

void* FindPatternInModule(HMODULE hModule, BYTE* pattern, SIZE_T patternSize) {
	if (hModule == nullptr) {
		return nullptr;
	}

	// Get the module information to determine its memory range
	MODULEINFO moduleInfo;
	if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO))) {
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

uintptr_t ScanSliceProcessMemory(const uint8_t* pattern, const char* mask, uintptr_t startAddress, uintptr_t endAddress) {
	size_t patternSize = strlen(mask);

	for (uintptr_t address = startAddress; address < endAddress; ) {
		MEMORY_BASIC_INFORMATION memInfo;
		if (VirtualQuery(reinterpret_cast<LPCVOID>(address), &memInfo, sizeof(memInfo)) == sizeof(memInfo)) {
			if (memInfo.State == MEM_COMMIT && (memInfo.Protect == PAGE_READWRITE || memInfo.Protect == PAGE_EXECUTE_READWRITE)) {
				size_t bufferSize = std::min<size_t>(memInfo.RegionSize, endAddress - address);

				std::vector<uint8_t> buffer(bufferSize);
				SIZE_T bytesRead;
				if (ReadProcessMemory(GetCurrentProcess(), memInfo.BaseAddress, buffer.data(), bufferSize, &bytesRead)) {
					for (size_t i = 0; i <= bytesRead - patternSize; ++i) {
						bool found = true;
						for (size_t j = 0; j < patternSize; ++j) {
							if (mask[j] != '?' && buffer[i + j] != pattern[j]) {
								found = false;
								break;
							}
						}
						if (found) {
							return address + i;
						}
					}
				}
			}

			address += memInfo.RegionSize;
		}
		else {
			address += 0x1000;
		}
	}

	return 0;
}


uintptr_t ScanProcessMemory(const uint8_t* pattern, const char* mask)
{
	return ScanSliceProcessMemory(pattern, mask, PSOBB_CODE_REGION_START, PSOBB_CODE_REGION_END);
}