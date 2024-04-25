#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include <Psapi.h>
#include <iostream>

#include <toml++/toml.hpp>
#include <detours/detours.h>

#include "../memory.h"
#include "../logging.h"
#include "patches.h"
#include "gameguard.h"
#include "language.h"
#include "input.h"
#include "misc.h"

#pragma intrinsic(_ReturnAddress)

// Store configuration specifics
bool canBypassGameGuard;
bool enableDirectInput;
bool isEpisode4Enabled;
int clientLanguage;

// Pointer to the original GetStartupInfoA
auto startupInfoA = static_cast<decltype(GetStartupInfoA)*>(GetStartupInfoA);

void WINAPI AZ_GetStartUpInfoA(LPSTARTUPINFOA lpStartupInfo)
{
	const auto caller = GetOwningModule(reinterpret_cast<uintptr_t>(_ReturnAddress()));
	if (caller == GetModuleHandleA("PsoBB.exe"))
	{
		if (canBypassGameGuard)
		{
			PatchPackedGameGuard();
		}

		// Patch the game language
		// If the config entry is commented, default to the original Japanese mode
		PatchPackedGameLanguage(clientLanguage);

		// Patch Direct Input
		// Normally Teth executbles have this built in, but this may be needed sometimes
		if (enableDirectInput)
		{
			PatchPackedDirectInput();
		}

		// Patch Eepisode 4 Status
		if (isEpisode4Enabled)
		{
			PatchPackedEpisode4State(isEpisode4Enabled);
		}
	}

	return startupInfoA(lpStartupInfo);
}

void DoMemoryPatches(toml::table config)
{
	canBypassGameGuard = config.at_path("patches.gameguard").value_or(false);
	enableDirectInput = config.at_path("patches.enable_direct_input").value_or(false);
	clientLanguage = config.at_path("patches.client_language").value_or(0);
	isEpisode4Enabled = config.at_path("patches.episode4_mode").value_or(false);

	// Hook into GetStartupInfoA to allow hooking into the packed EXE's memory
	DetourAttach(&reinterpret_cast<PVOID&>(startupInfoA), AZ_GetStartUpInfoA);

	// We're repeating our patches here, so they work in the unpacked EXE

	// Patch the unpacked EXE if possible, and the process
	if (canBypassGameGuard)
	{
		PatchUnpackedGameGuard();
		PatchGameGuardProcess();
	}

	// Patch the game language
	// If the config entry is commented, default to the original Japanese mode
	PatchUnpackedGameLanguage(clientLanguage);

	// Patch Direct Input
	// Normally Teth executbles have this built in, but this may be needed sometimes
	if (enableDirectInput)
	{
		PatchUnpackedDirectInput();
	}

	// Patch Eepisode 4 Status
	if (isEpisode4Enabled)
	{
		PatchUnpackedEpisode4State(isEpisode4Enabled);
	}
}