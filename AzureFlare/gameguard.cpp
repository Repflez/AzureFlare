#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include <iostream>
#include <cstring>  // For C string functions

#include <detours/detours.h>

#include "memory.h"
#include "gameguard.h"

#define NPGAMEMON_SUCCESS 0x755

// Signature for InitNPGameMon
// Notes: return value seems to be an unsigned int? some places say it's a DWORD, others unsigned int
// Signature from  https://github.com/fuzziqersoftware/newserv/issues/336#issuecomment-2028486289 and
// https://github.com/fatrolls/nProtect-GameGuard/blob/164940ba81d318c300234c75f04da1412b554621/old/NPGameLib.c#L22
BYTE InitNPGameMonSignature[] = { 0x8B, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0xE9, 0xCC, 0xCC, 0xCC, 0xCC, 0x90 };

// Signature for PreInitNPGameMonA
// Notes: From: https://github.com/fuzziqersoftware/newserv/issues/336#issuecomment-2028486289 and
// https://github.com/fatrolls/nProtect-GameGuard/blob/164940ba81d318c300234c75f04da1412b554621/old/NPGameLib.c#L25
BYTE PreInitNPGameMonASignature[] = { 0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0x53, 0x33, 0xDB, 0x3B, 0xC3, 0x74, 0x04 };

// Pointer to the original CreateProcessA
OriginalCreateProcessA originalCreateProcessA = CreateProcessA;

// Pointer to the original InitNPGameMon
OriginalGameGuardInit originalGameGuardInit = nullptr;

// GameGuard Process Name
const char* GameGuardProcessName = "GameGuard.des";

// Custom detour function for CreateProcessA
BOOL WINAPI CreateProcessGameGuard(
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
) {
	// Let the game "know" GameGuard was loaded
	if (strstr(lpApplicationName, GameGuardProcessName) != nullptr)
	{
		return TRUE;
	}

	// Call back the original CreateProcessA function
	return originalCreateProcessA(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
	);
}

unsigned int BypassGameGuardInit()
{
	return NPGAMEMON_SUCCESS;
}

void PatchGameGuard()
{
	// Get the base address of psobb.exe
	HMODULE hModule = GetModuleHandle(NULL);

	// Find the address for InitNPGameMon
	// TODO: Get the packed EXE to work as well
	void* InitNPGameMonAddress = FindPatternInModule(hModule, InitNPGameMonSignature, sizeof(InitNPGameMonSignature));
	if (InitNPGameMonAddress != nullptr)
	{
		originalGameGuardInit = reinterpret_cast<OriginalGameGuardInit>(InitNPGameMonAddress);

		// Attach our GameGuard Bypass
		DetourAttach(&reinterpret_cast<PVOID&>(originalGameGuardInit), BypassGameGuardInit);
	}

	// Detour CreateProcessA to bypass GameGuard
	DetourAttach(&reinterpret_cast<PVOID&>(originalCreateProcessA), CreateProcessGameGuard);
}

void UnpatchGameGuard()
{
	//DetourDetach(&(PVOID&)originalCreateProcessA, CreateProcessGameGuard);
	DetourDetach(&reinterpret_cast<PVOID&>(originalCreateProcessA), CreateProcessGameGuard);

	DetourDetach(&reinterpret_cast<PVOID&>(originalGameGuardInit), BypassGameGuardInit);
}