#include <console.hh>
#include <hook.hh>
#include <pattern.hh>

#include <settings/settings.hh>

#include "language.hh"

namespace AzureFlare::Patches
{
    void Language::Patch()
    {
        PRINT_DEBUG("Language: Attempting to patch Language");

        auto get_language = hook::pattern("E8 03 19 D9 FF");
        get_language.for_each_result([](hook::pattern_match i)
        {
            auto offset = i.get<std::uint32_t>();
            PRINT_DEBUG("Language: Found pattern for Language at 0x%p, patching", offset);
            
            Utils::hook::return_value(reinterpret_cast<std::uint32_t>(offset), Settings::CurrentLanguage);
        });

        get_language.clear();
    }
}