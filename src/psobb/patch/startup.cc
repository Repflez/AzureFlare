
#include "startup.hh"
#include "gameguard.hh"

#include <MinHook.h>

#include <console.hh>
#include <hook.hh>
#include <pattern.hh>

#include <settings/settings.hh>

#include "language.hh"
#include "episode4.hh"
#include "ime.hh"

namespace AzureFlare::Patches
{
    // Pointer to the original GetStartupInfoA
    decltype(GetStartupInfoA)* Startup::StartupInfoA = nullptr;

    void Startup::Patch()
    {
        const MH_STATUS init_rc = MH_Initialize();
        PRINT_DEBUG("Startup: MH_Initialize=%d", init_rc);

        if (init_rc != MH_OK && init_rc != MH_ERROR_ALREADY_INITIALIZED)
        {
            PRINT_WARNING("Startup: MH_Initialize failed, abort");
            return;
        }

        const MH_STATUS create_rc = MH_CreateHook(
            reinterpret_cast<LPVOID>(GetStartupInfoA),
            reinterpret_cast<LPVOID>(Startup::GetStartUpInfoA),
            reinterpret_cast<LPVOID*>(&StartupInfoA)
        );
        PRINT_DEBUG("Startup: MH_CreateHook=%d real=0x%p", (int)create_rc, (void*)StartupInfoA);
        if (create_rc != MH_OK) return;

        const MH_STATUS enable_rc = MH_EnableHook(reinterpret_cast<LPVOID>(GetStartupInfoA));
        PRINT_DEBUG("Startup: MH_EnableHook=%d", (int)enable_rc);
        if (enable_rc != MH_OK) return;

        PRINT_INFO("Startup: success");

        if (Settings::DisableGameGuard) GameGuard::PatchCreateProcess();
    }

    void WINAPI Startup::GetStartUpInfoA(LPSTARTUPINFOA lpStartupInfo)
    {
        const auto caller = GetOwningModule(reinterpret_cast<uintptr_t>(RETURN_ADDRESS()));
        
        if (caller == GetModuleHandleA("PsoBB.exe"))
        {
            if (Settings::DisableGameGuard) Patches::GameGuard::Patch();
            Patches::Language::Patch();
            Patches::Episode4::Patch();
            Patches::IME::Patch();
        }

        return StartupInfoA(lpStartupInfo);
    }

    inline HMODULE WINAPI Startup::GetOwningModule(const uintptr_t address)
    {
        if (address == 0)
            return nullptr;

        MEMORY_BASIC_INFORMATION mbi{};
        if (VirtualQuery((LPCVOID)address, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
            return (HMODULE)mbi.AllocationBase;

        return nullptr;
    }
}