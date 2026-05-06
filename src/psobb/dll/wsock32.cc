#include <console.hh>
#include <settings/settings.hh>

#include "wsock32.hh"

using namespace AzureFlare;

FARPROC __WSAFDIsSet;
FARPROC closesocket;
FARPROC connect;
FARPROC gethostbyaddr;
FARPROC getsockopt;
FARPROC htons;
FARPROC inet_addr;
FARPROC recv;
FARPROC recvfrom;
FARPROC select;
FARPROC send;
FARPROC sendto;
FARPROC setsockopt;
FARPROC shutdown;
FARPROC socket;
FARPROC WSACleanup;
FARPROC WSAGetLastError;
FARPROC WSAStartup;

extern "C" NAKED void ___WSAFDIsSet() { jmp(__WSAFDIsSet); }
extern "C" NAKED void _closesocket()  { jmp(closesocket); }
extern "C" NAKED void _connect()  { jmp(connect); }
extern "C" NAKED void _gethostbyaddr()  { jmp(gethostbyaddr); }
extern "C" NAKED void _getsockopt()  { jmp(getsockopt); }
extern "C" NAKED void _htons()  { jmp(htons); }
extern "C" NAKED void _inet_addr()  { jmp(inet_addr); }
extern "C" NAKED void _recv()  { jmp(recv); }
extern "C" NAKED void _recvfrom()  { jmp(recvfrom); }
extern "C" NAKED void _select()  { jmp(select); }
extern "C" NAKED void _send()  { jmp(send); }
extern "C" NAKED void _sendto()  { jmp(sendto); }
extern "C" NAKED void _setsockopt()  { jmp(setsockopt); }
extern "C" NAKED void _shutdown()  { jmp(shutdown); }
extern "C" NAKED void _socket()  { jmp(socket); }
extern "C" NAKED void _WSACleanup()  { jmp(WSACleanup); }
extern "C" NAKED void _WSAGetLastError()  { jmp(WSAGetLastError); }
extern "C" NAKED void _WSAStartup()  { jmp(WSAStartup); }

typedef struct hostent* (WINAPI* OriginalGetHostByName)(const char*);
OriginalGetHostByName pOriginalGetHostByName = nullptr;

void HookLibraryFunctions()
{
    __WSAFDIsSet = GetProcAddress(hDll, "__WSAFDIsSet");
    closesocket = GetProcAddress(hDll, "closesocket");
    connect = GetProcAddress(hDll, "connect");
    gethostbyaddr = GetProcAddress(hDll, "gethostbyaddr");
    getsockopt = GetProcAddress(hDll, "getsockopt");
    htons = GetProcAddress(hDll, "htons");
    inet_addr = GetProcAddress(hDll, "inet_addr");
    recv = GetProcAddress(hDll, "recv");
    recvfrom = GetProcAddress(hDll, "recvfrom");
    select = GetProcAddress(hDll, "select");
    send = GetProcAddress(hDll, "send");
    sendto = GetProcAddress(hDll, "sendto");
    setsockopt = GetProcAddress(hDll, "setsockopt");
    shutdown = GetProcAddress(hDll, "shutdown");
    socket = GetProcAddress(hDll, "socket");
    WSACleanup = GetProcAddress(hDll, "WSACleanup");
    WSAGetLastError = GetProcAddress(hDll, "WSAGetLastError");
    WSAStartup = GetProcAddress(hDll, "WSAStartup");

    // Load the gethostbyname function separately
	pOriginalGetHostByName = reinterpret_cast<OriginalGetHostByName>(GetProcAddress(hDll, "gethostbyname"));
}

#define REPLACE_SERVER_URL(outBuffer, passedValue, origAddr, newAddr, comment) \
if (newAddr == NULL) \
{ \
    PRINT_DEBUG_N("New hostname for %s is null, using %s", comment, passedValue); \
    strncpy(outBuffer, passedValue, sizeof(outBuffer) - 1); \
    outBuffer[sizeof(outBuffer) - 1] = '\0'; \
} \
else if (strcmp(passedValue, origAddr) == 0) \
{ \
    strncpy(outBuffer, newAddr, sizeof(outBuffer) - 1); \
    outBuffer[sizeof(outBuffer) - 1] = '\0'; \
}

extern "C" hostent* __stdcall _gethostbyname(const char* name)
{
    if (!Settings::EnableServerRedirection) return pOriginalGetHostByName(name);

	char newHostname[256] = "";

	// US Servers
	REPLACE_SERVER_URL(newHostname, name, "patch01.us.segaonline.jp", Settings::GameUrls.USAServerUrls.PatchServerUrl.c_str(), "US Patch Server");
	REPLACE_SERVER_URL(newHostname, name, "game01.us.segaonline.jp", Settings::GameUrls.USAServerUrls.GameServerUrl.c_str(), "US Game Server");

	// JP Servers
	REPLACE_SERVER_URL(newHostname, name, "patch01.psobb.segaonline.jp", Settings::GameUrls.JPServerUrls.PatchServerUrl.c_str(), "JP Patch Server");
	REPLACE_SERVER_URL(newHostname, name, "game01.psobb.segaonline.jp", Settings::GameUrls.JPServerUrls.GameServerUrl.c_str(), "JP Game Server");

	// Episode 4 Servers
	REPLACE_SERVER_URL(newHostname, name, "psobb-ep4-patch.segaonline.jp", Settings::GameUrls.EP4ServerUrls.PatchServerUrl.c_str(), "Ep4 Patch Server");
	REPLACE_SERVER_URL(newHostname, name, "psobb-ep4-db.segaonline.jp", Settings::GameUrls.EP4ServerUrls.GameServerUrl.c_str(), "Ep4 Game Server");

	// CN Servers
	REPLACE_SERVER_URL(newHostname, name, "patch.psobb.cn", Settings::GameUrls.CNServerUrls.PatchServerUrl.c_str(), "CN Patch Server");
	REPLACE_SERVER_URL(newHostname, name, "db.psobb.cn", Settings::GameUrls.CNServerUrls.GameServerUrl.c_str(), "CN Game Server");

	PRINT_DEBUG_N("Hostname: %s => %s", name, newHostname);

	return pOriginalGetHostByName(newHostname);
}