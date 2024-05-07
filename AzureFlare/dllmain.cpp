#define WIN32_LEAN_AND_MEAN
#define TOML_ENABLE_FORMATTERS 0

#include <windows.h>
#include <winsock.h>
#include <stdio.h>

#include <filesystem>

#include <detours/detours.h>
#include <toml++/toml.hpp>

#include "logging.h"
#include "patches/patches.h"

using namespace std::string_view_literals;

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

#pragma region Server Urls

// Server Urls

// US Servers
const char* patchServerUsUrl = "patch01.us.segaonline.jp";
const char* gameServerUsUrl = "game01.us.segaonline.jp";

// JP Non Ep4 Servers
const char* patchServerJpUrl = "patch01.psobb.segaonline.jp";
const char* gameServerJpUrl = "game01.psobb.segaonline.jp";

// JP Ep4 Urls
// Notes: EP4 was released separately in Japan, it was bundled elsewhere as Blue Burst
const char* ep4PatchServerUrl = "psobb-ep4-patch.segaonline.jp";
const char* ep4GameServerUrl = "psobb-ep4-db.segaonline.jp";

// CN Servers
// Notes: There's some psobb.cn domains in the EXE
const char* patchServerCnUrl = "patch.psobb.cn";
const char* gameServerCnUrl = "db.psobb.cn";

// Fallback, in case the property is missing from the config file
const char* fallbackUrl = "localhost";

#pragma endregion



FARPROC p[ALLFUNC_COUNT] = { 0 };

HMODULE hDLL;

typedef struct hostent* (WINAPI* OriginalGetHostByName)(const char*);
OriginalGetHostByName pOriginalGetHostByName = nullptr;

bool loaded = FALSE;
bool canRedirectServers = TRUE;
toml::table config;

#define AF_SERVER_REDIRECT(outBuffer, passedValue, origAddr, newAddr, comment) \
if (strcmp(passedValue, origAddr) == 0) \
{ \
    strncpy_s(outBuffer, newAddr, strnlen_s(newAddr, sizeof(outBuffer))); \
	DLOG("Redirecting %s to %s\n", comment, newAddr); \
}

// Note: Performance is a non issue here as it's only called twice: Patch Screen and Connection screen.
extern "C" hostent * __stdcall _AZFLARE_EXPORT_gethostbyname(char* name)
{
	if (!loaded || !canRedirectServers)
		return pOriginalGetHostByName(name);

	char buf[256] = "";

	// Fallback in case no matches are found. See Teth EXEs using IP Addresses.
	AF_SERVER_REDIRECT(buf, name, name, name, "Fallback");

	// US Servers
	AF_SERVER_REDIRECT(buf, name, patchServerUsUrl, config.at_path("redirect.us.patch_server").value_or(fallbackUrl), "US Patch Server");
	AF_SERVER_REDIRECT(buf, name, gameServerUsUrl, config.at_path("redirect.us.game_server").value_or(fallbackUrl), "US Game Server");

	// JP Servers
	AF_SERVER_REDIRECT(buf, name, patchServerJpUrl, config.at_path("redirect.jp.patch_server").value_or(fallbackUrl), "JP Patch Server");
	AF_SERVER_REDIRECT(buf, name, gameServerJpUrl, config.at_path("redirect.jp.game_server").value_or(fallbackUrl), "JP Game Server");

	// Episode 4 Servers
	AF_SERVER_REDIRECT(buf, name, ep4PatchServerUrl, config.at_path("redirect.ep4.patch_server").value_or(fallbackUrl), "Ep4 Patch Server");
	AF_SERVER_REDIRECT(buf, name, ep4GameServerUrl, config.at_path("redirect.ep4.game_server").value_or(fallbackUrl), "Ep4 Game Server");

	// CN Servers
	AF_SERVER_REDIRECT(buf, name, patchServerCnUrl, config.at_path("redirect.cn.patch_server").value_or(fallbackUrl), "CN Patch Server");
	AF_SERVER_REDIRECT(buf, name, gameServerCnUrl, config.at_path("redirect.cn.game_server").value_or(fallbackUrl), "CN Game Server");

	return pOriginalGetHostByName(buf);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		const char* configFilename = "psobb.cfg";
		char bufd[200], buf[256];

#if _DEBUG
		SetConsoleOutputCP(65001);
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		freopen("CON", "w", stdout);
#endif

		// Check if the configuration exists
		if (!std::filesystem::exists(configFilename))
		{
			sprintf_s(buf, sizeof(buf), "%s could not be found. Please ensure it exists.\nThe game will close now.", configFilename);
			MessageBoxA(NULL, buf, "AzureFlare Config Error", MB_OK);
			ExitProcess(1);
		}

		// Load configuration file
		try
		{
			config = toml::parse_file(configFilename);
		}
		catch (toml::parse_error& _)
		{
			MessageBoxA(NULL, "There was an error loading the configuration. Please check the file.\nThe game will close now.", "AzureFlare Config Error", MB_OK);
			ExitProcess(1);
		}

		GetSystemDirectoryA(bufd, sizeof(bufd));
		strcat_s(bufd, "\\WSOCK32.dll");

		hDLL = LoadLibraryA(bufd);
		if (!hDLL) return false;

		// Load the addresses for all the functions
#define REGISTER(num, name, ordinal) p[num] = GetProcAddress(hDLL, #name);
		ALLFUNC(REGISTER);
#undef REGISTER

		// Load the gethostbyname function separately
		pOriginalGetHostByName = reinterpret_cast<OriginalGetHostByName>(GetProcAddress(hDLL, "gethostbyname"));

		// Set the server redirection patch status here now, the rest are handled elsewhere
		canRedirectServers = config.at_path("patches.redirect").value_or(false);

		DLOG(canRedirectServers ? "Server redirection will be enabled\n" : "Server redirection will not be enabled\n");

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		// Do our patches now
		DoMemoryPatches(config);

		DetourTransactionCommit();

		loaded = TRUE;
	}

	if (fdwReason == DLL_PROCESS_DETACH)
	{
		pOriginalGetHostByName = nullptr;

		// Clean the detours we made
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		// Finalize the transaction
		DetourTransactionCommit();
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