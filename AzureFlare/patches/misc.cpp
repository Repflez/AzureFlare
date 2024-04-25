#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <detours/detours.h>

#include "../logging.h"
#include "../memory.h"

#include "misc.h"

// Signature for the function that decides if the game is Episode 4 or not
BYTE IsEp4Signature[] = { 0xB8, 0x30, 0x11, 0x00, 0x00 };

bool IsEpisode4Enabled;

int IsEp4Mode()
{
	return IsEpisode4Enabled;
}

void PatchPackedEpisode4State(bool enabled)
{
	IsEpisode4Enabled = enabled;

	uintptr_t address = ScanProcessMemory(IsEp4Signature, "xxxxx");
	if (address != 0)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&reinterpret_cast<PVOID&>(address), IsEp4Mode);
		DetourTransactionCommit();
	}
}

void PatchUnpackedEpisode4State(bool enabled)
{
	IsEpisode4Enabled = enabled;

	void* IsEp4Address = FindPatternInModule(GetModuleHandle(NULL), IsEp4Signature, sizeof(IsEp4Signature));
	if (IsEp4Address != nullptr)
	{
		DetourAttach(&reinterpret_cast<PVOID&>(IsEp4Address), IsEp4Mode);
	}
}