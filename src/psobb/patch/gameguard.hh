#pragma once

namespace AzureFlare::Patches
{
    class GameGuard
    {
    public:
        static void Patch();
        static void PatchCreateProcess();
    private:
        static bool WINAPI _CreateProcessA(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES,
            LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA,
            LPPROCESS_INFORMATION);
    };
}