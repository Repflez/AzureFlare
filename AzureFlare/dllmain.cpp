#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#include <stdio.h>

#pragma pack(1)

#pragma region ALLFUNC(FUNC)

#define ALLFUNC(FUNC) \
	FUNC(0,__WSAFDIsSet,151) \
	FUNC(1,closesocket,3) \
	FUNC(2,connect,4) \
	FUNC(3,gethostbyaddr,51) \
	FUNC(4,getsockopt,7) \
	FUNC(5,htons,9) \
	FUNC(6,inet_addr,10) \
	FUNC(7,recv,16) \
	FUNC(8,recvfrom,17) \
	FUNC(9,select,18) \
	FUNC(10,send,19) \
	FUNC(11,sendto,20) \
	FUNC(12,setsockopt,21) \
	FUNC(13,shutdown,22) \
	FUNC(14,socket,23) \
	FUNC(15,WSACleanup,116) \
	FUNC(16,WSAGetLastError,111) \
	FUNC(17,WSAIsBlocking,114) \
	FUNC(18,WSAStartup,115) \

#define ALLFUNC_COUNT 19

#pragma endregion

FARPROC p[ALLFUNC_COUNT] = { 0 };

HMODULE hDLL;

typedef struct hostent* (WINAPI* OriginalGetHostByName)(const char*);
OriginalGetHostByName pOriginalGetHostByName = nullptr;

extern "C" hostent * __stdcall _AZFLARE_EXPORT_gethostbyname(char* name)
{
	hostent* hostnameResult;

	// TODO: Configurable redirect
	hostnameResult = pOriginalGetHostByName((char*)"127.0.0.1");

	return hostnameResult;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		//DisableThreadLibraryCalls(hinstDLL);

		char bufd[200];
		GetSystemDirectoryA(bufd, 200);
		strcat_s(bufd, "\\WSOCK32.dll");

		hDLL = LoadLibraryA(bufd);
		if (!hDLL) return false;

		// Load the addresses for all the functions
#define REGISTER(num, name, ordinal) p[num] = GetProcAddress(hDLL, #name);
		ALLFUNC(REGISTER);
#undef REGISTER

		// Load the gethostbyname function separately
		pOriginalGetHostByName = reinterpret_cast<OriginalGetHostByName>(GetProcAddress(hDLL, "gethostbyname"));
	}

	if (fdwReason == DLL_PROCESS_DETACH)
	{
		pOriginalGetHostByName = nullptr;
		FreeLibrary(hDLL);
	}

	return TRUE;
}

#define DEF_STUB(num, name, ordinal) \
extern "C" __declspec(naked) void __stdcall _AZFLARE_EXPORT_ ## name() \
{ \
	__asm \
	{ \
		jmp p[num * 4] \
	} \
};\

ALLFUNC(DEF_STUB)
#undef DEF_STUB