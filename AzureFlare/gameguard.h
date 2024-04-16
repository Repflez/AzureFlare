#pragma once
#include <windows.h>

// Typedef for the original CreateProcessA function
typedef BOOL(WINAPI* OriginalCreateProcessA)(
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
	);

// Typedef for the original InitNPGameMon
typedef unsigned int (*OriginalGameGuardInit)();

// Typedef for the original PreInitNPGameMonA
typedef int __cdecl OriginalPreInitNPGameMonA(char* a1);

void PatchGameGuard();
void UnpatchGameGuard();