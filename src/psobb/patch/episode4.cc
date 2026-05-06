#include <console.hh>
#include <hook.hh>
#include <pattern.hh>

#include <settings/settings.hh>

#include "episode4.hh"

namespace AzureFlare::Patches
{
    void Episode4::Patch()
    {
        PRINT_DEBUG("Episode4: Attempting to patch Episode4");

        auto episode4 = hook::pattern("B8 30 11 00 00");
        episode4.for_each_result([](hook::pattern_match i)
        {
            auto offset = i.get<std::uint32_t>();
            PRINT_DEBUG("Episode4: Found pattern for Episode4 at 0x%p, patching", offset);
            
            Utils::hook::return_value(reinterpret_cast<std::uint32_t>(offset), Settings::EnableEpisode4Mode);
        });

        episode4.clear();
    }
}