#pragma once

namespace AzureFlare::Patches
{
    class Startup
    {
    public:
        static void Patch();
    private:
        static decltype(GetStartupInfoA)* StartupInfoA;

        static void WINAPI GetStartUpInfoA(LPSTARTUPINFOA);
        static HMODULE WINAPI GetOwningModule(const uintptr_t address);
    };
}