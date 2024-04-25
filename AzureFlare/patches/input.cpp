#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../logging.h"
#include "../memory.h"

#include "input.h"

constexpr auto INPUT_ADDRESS_OFFSET = 0x56;
constexpr auto PATCHED_CALL = 0x008EC39C;

// Signature for the chat IME function
BYTE IMESignature[] = { 0x55, 0x8B, 0xEC, 0x6A, 0xFF, 0x68, 0x88, 0x6E, 0x99, 0x00 };

static void PatchDirectInput(uintptr_t address)
{
	int FinalAddress = address + INPUT_ADDRESS_OFFSET;

	// Actually patch input by using another instruction
	// Personally I have no idea why this works, but it's in Solybum's repo, so I'm using it.
	// Thanks man!
	*(int*)FinalAddress = PATCHED_CALL;
}

void PatchPackedDirectInput()
{
	uintptr_t address = ScanProcessMemory(IMESignature, "xxxxxxxxxx");
	if (address != 0)
	{
		PatchDirectInput(address);
	}
}

void PatchUnpackedDirectInput()
{
	void* DirectInputAddress = FindPatternInModule(GetModuleHandle(NULL), IMESignature, sizeof(IMESignature));
	if (DirectInputAddress != nullptr)
	{
		PatchDirectInput(reinterpret_cast<uintptr_t>(DirectInputAddress));
	}
}