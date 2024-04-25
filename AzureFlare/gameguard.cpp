#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <intrin.h>
#include <Psapi.h>
#include <iostream>

#include <toml++/toml.hpp>
#include <detours/detours.h>

#include "logging.h"
#include "signatures.h"
#include "memory.h"
#include "gameguard.h"

#pragma intrinsic(_ReturnAddress)

#define NPGAMEMON_SUCCESS 0x755

#define PSOBB_REGION_START 0x401000
#define PSOBB_REGION_END PSOBB_REGION_START + 0x638000

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

// Pointer to the original PreInitNPGameMonA
OriginalPreInitNPGameMonA originalPreInitGameGuard = nullptr;

// Pointer to the original game region function
OriginalIsEp4 originalIsEp4Address = nullptr;

OriginalGetGameLanguage originalGameLanguage = nullptr;

// Pointer to the original GetStartupInfoA
auto startupInfoA = static_cast<decltype(GetStartupInfoA)*>(GetStartupInfoA);

// GameGuard Process Name
const char* GameGuardProcessName = "GameGuard.des";

toml::table az_config;

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

int __cdecl BypassGameGuardPreInit(char* a1)
{
	// Seems to work if I send any non-zero value, or 0
	// I'm sending a non-zero value for now.
	return 1;
}

int GetGameLanguage() {
	// 0: Japanese (j)
	// 1: English (e)
	// 2: German (g)
	// 3: French (f)
	// 4: Spanish (s)
	// 5: Chinese Simplified (cs)
	// 6: Chinese Traditional (ct)
	// 7: Korean (h)
	return 1;
}

int IsEp4()
{
	return az_config.at_path("patches.episode4_mode").value_or(false);
}

void ApplyGameGuardPatches(HMODULE hModule)
{
	// Find the address for InitNPGameMon
	void* InitNPGameMonAddress = FindPatternInModule(hModule, InitNPGameMonSignature, sizeof(InitNPGameMonSignature));
	if (InitNPGameMonAddress != nullptr)
	{
		originalGameGuardInit = reinterpret_cast<OriginalGameGuardInit>(InitNPGameMonAddress);

		// Attach our GameGuard Bypass
		DetourAttach(&reinterpret_cast<PVOID&>(originalGameGuardInit), BypassGameGuardInit);
	}

	// Find the address for PreInitNPGameMonA
	// Notes: Bypassing this function will avoid generating the GameGuard folder and its sole npgl.erl file
	void* PreInitNPGameMonAddress = FindPatternInModule(hModule, PreInitNPGameMonASignature, sizeof(PreInitNPGameMonASignature));
	if (PreInitNPGameMonAddress != nullptr)
	{
		originalPreInitGameGuard = reinterpret_cast<OriginalPreInitNPGameMonA>(PreInitNPGameMonAddress);

		// Attach our second GameGuard Bypass
		DetourAttach(&reinterpret_cast<PVOID&>(originalPreInitGameGuard), BypassGameGuardPreInit);
	}
}

void WINAPI AZ_GetStartUpInfoA(LPSTARTUPINFOA lpStartupInfo)
{
	const auto caller = GetOwningModule(reinterpret_cast<uintptr_t>(_ReturnAddress()));
	if (caller == GetModuleHandleA("PsoBB.exe"))
	{
		uintptr_t address = ScanProcessMemory(InitNPGameMonSignature, "xx????xxxxxxxx????x", PSOBB_REGION_START, PSOBB_REGION_END);

		// Restarting the entire detours process makes it work. No idea why.
		if (address != 0) {
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			originalGameGuardInit = reinterpret_cast<OriginalGameGuardInit>(address);
			DetourAttach(&reinterpret_cast<PVOID&>(originalGameGuardInit), BypassGameGuardInit);

			DetourTransactionCommit();
		}

		address = ScanProcessMemory(PreInitNPGameMonASignature, "x????xxxxxxx", PSOBB_REGION_START, PSOBB_REGION_END);
		if (address != 0) {
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			originalPreInitGameGuard = reinterpret_cast<OriginalPreInitNPGameMonA>(address);
			DetourAttach(&reinterpret_cast<PVOID&>(originalPreInitGameGuard), BypassGameGuardPreInit);

			DetourTransactionCommit();
		}

		// Patch episode 4 data now
		address = ScanProcessMemory(IsEp4Signature, "xxxxx", PSOBB_REGION_START, PSOBB_REGION_END);
		if (address != 0) {
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			originalIsEp4Address = reinterpret_cast<OriginalIsEp4>(address);
			DetourAttach(&reinterpret_cast<PVOID&>(originalIsEp4Address), IsEp4);

			DetourTransactionCommit();
		}

		// Game Language
		address = ScanProcessMemory(GameLanguageSignature, "xxxxx", PSOBB_REGION_START, PSOBB_REGION_END);
		if (address != 0) {
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			originalGameLanguage = reinterpret_cast<OriginalGetGameLanguage>(address);
			DetourAttach(&reinterpret_cast<PVOID&>(originalGameLanguage), GetGameLanguage);

			DetourTransactionCommit();
		}


		address = ScanProcessMemory(IMESignature, "xxxxxxxxxx", PSOBB_REGION_START, PSOBB_REGION_END);
		if (address != 0) {
			int FinalAddress = address + 0x56;

			*(int*)FinalAddress = 0x008EC39C;
		}
	}

	return startupInfoA(lpStartupInfo);
}


#ifdef _DEBUG
auto handleFIleA = static_cast<decltype(CreateFileA)*>(CreateFileA);
HANDLE WINAPI MyCreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile)
{
	DLOG("CreateFileA: %s\n", lpFileName);
	auto demhandle = handleFIleA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	return demhandle;
}
#endif

void PatchGameGuard(toml::table config)
{
	az_config = config;

#ifdef _DEBUG
	DetourAttach(&reinterpret_cast<PVOID&>(handleFIleA), MyCreateFileA);
#endif

	// Patch the GameGuard functions for unpacked versions of the client
	ApplyGameGuardPatches(GetModuleHandle(NULL));

	// Hook into GetStartupInfoA to allow using the packed EXE
	DetourAttach(&reinterpret_cast<PVOID&>(startupInfoA), AZ_GetStartUpInfoA);

	// Detour CreateProcessA to bypass GameGuard
	DetourAttach(&reinterpret_cast<PVOID&>(originalCreateProcessA), CreateProcessGameGuard);
}

void UnpatchGameGuard()
{
	DetourDetach(&reinterpret_cast<PVOID&>(originalCreateProcessA), CreateProcessGameGuard);

	// Remove our GameGuard bypasses, at this point the game is closing, so we shouldn't worry about null pointers here?
	DetourDetach(&reinterpret_cast<PVOID&>(originalGameGuardInit), BypassGameGuardInit);
	DetourDetach(&reinterpret_cast<PVOID&>(originalPreInitGameGuard), BypassGameGuardPreInit);
}