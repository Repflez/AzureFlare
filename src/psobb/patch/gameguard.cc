#include "gameguard.hh"

#include <console.hh>
#include <hook.hh>
#include <pattern.hh>

#include <MinHook.h>

#define NPGAMEMON_SUCCESS 0x755

namespace AzureFlare::Patches
{
    // Pointer to the original CreateProcessA
    decltype(CreateProcessA)* pCreateProcessA = nullptr;
    
    void GameGuard::Patch()
    {
        PRINT_DEBUG("GameGuard: Attempting to patch GameGuard");

        auto init_np_game_mon = hook::pattern("8B 0D ? ? ? ? 85 C9 75 03 33 C0 C3 E9 ? ? ? ? 90");
        init_np_game_mon.for_each_result([](hook::pattern_match i)
        {
            auto offset = i.get<std::uint32_t>();
            PRINT_DEBUG("GameGuard: Found pattern for InitNPGameMon at 0x%p, patching", offset);
            
            Utils::hook::return_value(reinterpret_cast<std::uint32_t>(offset), NPGAMEMON_SUCCESS);
        });
        
        auto pre_init_np_game_mon_a = hook::pattern("A1 ? ? ? ? 53 33 DB 3B C3 74 04");
        pre_init_np_game_mon_a.for_each_result([](hook::pattern_match i)
        {
            auto offset = i.get<std::uint32_t>();
            PRINT_DEBUG("GameGuard: Found pattern for PreInitNPGameMonA 0x%p, patching", offset);
            
            Utils::hook::return_value(reinterpret_cast<std::uint32_t>(offset), 1);
        });

        init_np_game_mon.clear();
        pre_init_np_game_mon_a.clear();
    }

    void GameGuard::PatchCreateProcess()
    {
        const MH_STATUS init_rc = MH_Initialize();
        PRINT_DEBUG("CreateProcess: MH_Initialize=%d", init_rc);

        if (init_rc != MH_OK && init_rc != MH_ERROR_ALREADY_INITIALIZED)
        {
            PRINT_WARNING("CreateProcess: MH_Initialize failed, abort");
            return;
        }

        const MH_STATUS create_rc = MH_CreateHook(
            reinterpret_cast<LPVOID>(CreateProcessA),
            reinterpret_cast<LPVOID>(GameGuard::_CreateProcessA),
            reinterpret_cast<LPVOID*>(&pCreateProcessA)
        );

        PRINT_DEBUG("CreateProcess: MH_CreateHook=%d real=0x%p", (int)create_rc, (void*)pCreateProcessA);
        if (create_rc != MH_OK) return;

        const MH_STATUS enable_rc = MH_EnableHook(reinterpret_cast<LPVOID>(CreateProcessA));
        PRINT_DEBUG("CreateProcess: MH_EnableHook=%d", (int)enable_rc);
        if (enable_rc != MH_OK) return;

        PRINT_INFO("CreateProcess: success");
    }

    // Function to not load GameGuard's main process
    bool WINAPI GameGuard::_CreateProcessA(
        LPCSTR lpApplicationName,
        LPSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles,
        DWORD dwCreationFlags,
        LPVOID lpEnvironment,
        LPCSTR lpCurrentDirectory,
        LPSTARTUPINFOA lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation)
    {
        PRINT_DEBUG("Checking CreateProcessA");

        // Let the game "know" GameGuard was loaded
        //if (strstr(lpApplicationName, "GameGuard.des") != nullptr)
        if (!_stricmp(lpApplicationName, "GameGuard.des"))
        {
            return true;
        }

        // Call back the original CreateProcessA function
        return pCreateProcessA(
            lpApplicationName,
            lpCommandLine,
            lpProcessAttributes,
            lpThreadAttributes,
            bInheritHandles,
            dwCreationFlags,
            lpEnvironment,
            lpCurrentDirectory,
            lpStartupInfo,
            lpProcessInformation
        );
    }
}