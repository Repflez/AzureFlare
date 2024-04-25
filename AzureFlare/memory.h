#pragma once
#include <windows.h>

#include <vector>

void* FindPatternInModule(HMODULE hModule, BYTE* pattern, SIZE_T patternSize);
uintptr_t ScanSliceProcessMemory(const uint8_t* pattern, const char* mask, uintptr_t startAddress, uintptr_t endAddress);
uintptr_t ScanProcessMemory(const uint8_t* pattern, const char* mask);


static inline HMODULE __stdcall GetOwningModule(const uintptr_t address)
{
	if (address == 0)
		return nullptr;

	MEMORY_BASIC_INFORMATION mbi{};
	if (VirtualQuery((LPCVOID)address, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
		return (HMODULE)mbi.AllocationBase;

	return nullptr;
}