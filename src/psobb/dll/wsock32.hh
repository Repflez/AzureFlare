#pragma once

#define jmp(a) { asm volatile ("jmp *%0" : : "m"(a)); }

extern HMODULE hDll;
void HookLibraryFunctions();