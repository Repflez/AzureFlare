#include <console.hh>
#include <hook.hh>
#include <pattern.hh>

#include <settings/settings.hh>

#include "ime.hh"

constexpr auto INPUT_ADDRESS_OFFSET = 0x56;
constexpr auto PATCHED_CALL = 0x008EC39C;

namespace AzureFlare::Patches
{
    void IME::Patch()
    {
        PRINT_DEBUG("IME: Attempting to patch IME");

        auto ime = hook::pattern("55 8B EC 6A FF 68 88 6E 99 00");
        ime.for_each_result([](hook::pattern_match i)
        {
            auto offset = i.get<std::uint32_t>();
            PRINT_DEBUG("IME: Found pattern for IME at 0x%p, patching", offset);

            int finalOffset = reinterpret_cast<std::uint32_t>(offset) + INPUT_ADDRESS_OFFSET;
            
            if (Settings::DisableIMEInput) Utils::hook::write(finalOffset, { 0x00, 0x8E, 0xC3, 0x9C }); // call ds:dword_8EC39C
        });

        ime.clear();
    }
}