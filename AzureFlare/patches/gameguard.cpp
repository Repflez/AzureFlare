#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <detours/detours.h>

#include "../logging.h"
#include "../memory.h"
#include "gameguard.h"

#define NPGAMEMON_SUCCESS 0x755

// TODO: Get this based on the EXE, but these correspond to JPBB 1.25.11
#define PSOBB_CODE_REGION_START 0x401000
#define PSOBB_CODE_REGION_END PSOBB_CODE_REGION_START + 0x638000

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
auto originalCreateProcessA = static_cast<decltype(CreateProcessA)*>(CreateProcessA);

// GameGuard Process Name
const char* GameGuardProcessName = "GameGuard.des";

// Function to not load GameGuard's main process
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
	LPPROCESS_INFORMATION lpProcessInformation)
{
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

int __cdecl BypassGameGuardPreInit(char* a1)
{
	// Seems to work if I send any non-zero value, or 0
	// I'm sending a non-zero value for now.
	return 1;
}

void PatchGameGuardProcess()
{
	DLOG("Patching GameGuard Process\n");

	// Detour CreateProcessA to bypass GameGuard's process
	DetourAttach(&reinterpret_cast<PVOID&>(originalCreateProcessA), CreateProcessGameGuard);
}

// This is actually a bit more complex than the unpacked EXE, but the heavylifting is done in patches.cpp
void PatchPackedGameGuard()
{
	DLOG("Patching packed GameGuard functions\n");

	// Holder for both of the addresses for GameGuard
	uintptr_t address = ScanProcessMemory(InitNPGameMonSignature, "xx????xxxxxxxx????x");

	DLOG("Scanned for InitNPGameMon's signature\n");

	// Restarting the entire detours process makes it work. No idea why.
	if (address != 0) {
		DLOG("Found InitNPGameMon signature at %X\n", address);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&reinterpret_cast<PVOID&>(address), BypassGameGuardInit);
		DetourTransactionCommit();
	}

	DLOG("InitNPGameMon scanned and probably patched\n");

	address = ScanProcessMemory(PreInitNPGameMonASignature, "x????xxxxxxx");

	DLOG("Scanned for PreInitNPGameMonA's signature\n");

	if (address != 0) {
		DLOG("Found PreInitNPGameMon signature at %X\n", address);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&reinterpret_cast<PVOID&>(address), BypassGameGuardPreInit);
		DetourTransactionCommit();
	}
}

void PatchUnpackedGameGuard()
{
	DLOG("Patching unpacked GameGuard functions\n");

	auto hModule = GetModuleHandle(NULL);

	// Find the address for InitNPGameMon
	void* InitNPGameMonAddress = FindPatternInModule(hModule, InitNPGameMonSignature, sizeof(InitNPGameMonSignature));
	if (InitNPGameMonAddress != nullptr)
	{
		DLOG("Found unpacked InitNPGameMon signature at %p\n", InitNPGameMonAddress);

		// Attach our GameGuard Bypass
		DetourAttach(&reinterpret_cast<PVOID&>(InitNPGameMonAddress), BypassGameGuardInit);
	}

	// Find the address for PreInitNPGameMonA
	// Notes: Bypassing this function will avoid generating the GameGuard folder and its sole npgl.erl file
	void* PreInitNPGameMonAddress = FindPatternInModule(hModule, PreInitNPGameMonASignature, sizeof(PreInitNPGameMonASignature));
	if (PreInitNPGameMonAddress != nullptr)
	{
		DLOG("Found unpacked PreInitNPGameMon signature at %p\n", PreInitNPGameMonAddress);

		// Attach our second GameGuard Bypass
		DetourAttach(&reinterpret_cast<PVOID&>(PreInitNPGameMonAddress), BypassGameGuardPreInit);
	}
}