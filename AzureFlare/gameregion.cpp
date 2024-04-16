#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include <detours/detours.h>

#include "memory.h"
#include "gameregion.h"

// Signature for the function that decides if the game is Episode 4 or not
BYTE IsEpisode4Signature[] = { 0xB8, 0x30, 0x11, 0x00, 0x00 };

// Pointer to the original game region function
OriginalIsEpisode4 originalIsEpisode4Address = nullptr;

int IsEpisode4Value = 0;

int IsEpisode4()
{
	return IsEpisode4Value;
}

void PatchEpisode4Mode(int value)
{
	// Get the base address of psobb.exe
	HMODULE hModule = GetModuleHandle(NULL);

	// Set the value from the config
	IsEpisode4Value = value;

	void* IsEpisode4Address = FindPatternInModule(hModule, IsEpisode4Signature, sizeof(IsEpisode4Signature));
	if (IsEpisode4Address != nullptr)
	{
		originalIsEpisode4Address = reinterpret_cast<OriginalIsEpisode4>(IsEpisode4Address);
		DetourAttach(&reinterpret_cast<PVOID&>(originalIsEpisode4Address), IsEpisode4);
	}
}

void UnpatchEpisode4Mode()
{
	DetourDetach(&reinterpret_cast<PVOID&>(originalIsEpisode4Address), IsEpisode4);
}