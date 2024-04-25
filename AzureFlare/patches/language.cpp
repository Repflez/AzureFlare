#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <detours/detours.h>

#include "../logging.h"
#include "../memory.h"

#include "language.h"

// Signature for the game language function
BYTE GameLanguageSignature[] = { 0xE8, 0x03, 0x19, 0xD9, 0xFF };

int LanguageId;

int GetGameLanguage() {
	// 0: Japanese (j)
	// 1: English (e)
	// 2: German (g)
	// 3: French (f)
	// 4: Spanish (s)
	// 5: Chinese Simplified (cs)
	// 6: Chinese Traditional (ct)
	// 7: Korean (h)
	return LanguageId;
}

void PatchPackedGameLanguage(int GameLanguageId)
{
	LanguageId = GameLanguageId;

	DLOG("Patching packed game language function\n");

	uintptr_t address = ScanProcessMemory(GameLanguageSignature, "xxxxx");

	DLOG("Scanned for GetGameLanguage's signature\n");

	if (address != 0)
	{
		DLOG("Found packed GetGameLanguage at %X\n", address);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&reinterpret_cast<PVOID&>(address), GetGameLanguage);
		DetourTransactionCommit();
	}

	DLOG("GetGameLanguage scanned and probably patched\n");
}

void PatchUnpackedGameLanguage(int GameLanguageId)
{
	LanguageId = GameLanguageId;

	DLOG("Patching unpacked game language function\n");

	void* GameLanguageAddress = FindPatternInModule(GetModuleHandle(NULL), GameLanguageSignature, sizeof(GameLanguageSignature));
	if (GameLanguageAddress != nullptr)
	{
		DLOG("Found unpacked GetGameLanguage at %p\n", GameLanguageAddress);
		DetourAttach(&reinterpret_cast<PVOID&>(GameLanguageAddress), GetGameLanguage);
	}
}