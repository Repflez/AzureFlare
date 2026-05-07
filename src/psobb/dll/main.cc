#include <forward_list>

#include <console.hh>
#include <format.hh>

#include "wsock32.hh"
#include <settings/settings.hh>

#include <patch/startup.hh>

HMODULE hDll;

using namespace AzureFlare;

void LoadDll()
{
    char DllPath[MAX_PATH];

    GetEnvironmentVariableA("windir", DllPath, MAX_PATH);
    Utils::Format::strscat(DllPath, "\\System32\\WSOCK32.dll");

    hDll = LoadLibraryA(DllPath);

    // Load Settings
    Settings::Init();

    // Hook into dsound's functions
    HookLibraryFunctions();

    // And now do our patching
    Patches::Startup::Patch();
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);
            setlocale(LC_ALL, "");

#ifdef DEBUG
		    Utils::Console::Init();
#endif
            LoadDll();
        break;
        
        case DLL_PROCESS_DETACH:
            FreeLibrary(hDll);
        break;
    }

    return TRUE;
}