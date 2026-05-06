#include <io.hh>
#include <ini/reader.h>
#include <console.hh>

#include "settings.hh"

namespace AzureFlare
{
    bool Settings::DisableGameGuard = false;
    bool Settings::EnableServerRedirection = false;
    Settings::Language Settings::CurrentLanguage = Settings::Language::English;
    bool Settings::EnableEpisode4Mode = false;
    bool Settings::DisableIMEInput = false;

	Settings::PsoBBServerUrls Settings::GameUrls{
		{"patch01.us.segaonline.jp",		"game01.us.segaonline.jp"},		// PSOBB US
		{"patch01.psobb.segaonline.jp",		"game01.psobb.segaonline.jp"},	// PSOBB JP
		{"patch.psobb.cn",					"db.psobb.cn"},					// PSOBB CN
		{"psobb-ep4-patch.segaonline.jp",	"psobb-ep4-db.segaonline.jp"},	// PSOBB JP Ep4
	};

    void Settings::Init()
	{
		if (!Utils::IO::FileExists(CONFIG_FILENAME))
		{
			Settings::CreateNewConfigFile();
		}

		Settings::LoadConfigurationFile();
	}

    void Settings::LoadConfigurationFile()
    {
        CIniReader config(CONFIG_FILENAME);

        int clientLanguage = config.ReadInteger("patches", "client_language", 1);
		if (clientLanguage >= 0 && clientLanguage <= 7)
		{
			PRINT_DEBUG_N("Language in config is %d", clientLanguage);
			Settings::CurrentLanguage = static_cast<Settings::Language>(clientLanguage);
		}
		else
		{
			Settings::CurrentLanguage = Settings::Language::English;
		}

        Settings::DisableGameGuard = config.ReadBoolean("patches", "gameguard", false);
        Settings::EnableServerRedirection = config.ReadBoolean("patches", "redirect", false);
        Settings::EnableEpisode4Mode = config.ReadBoolean("patches", "episode4_mode", false);
        Settings::DisableIMEInput = config.ReadBoolean("patches", "enable_direct_input", false);

        // Server URLs
		Settings::GameUrls.USAServerUrls.PatchServerUrl = config.ReadString("redirect.us", "patch_server", "patch01.us.segaonline.jp");
		Settings::GameUrls.USAServerUrls.GameServerUrl = config.ReadString("redirect.us", "game_server", "game01.us.segaonline.jp");
		Settings::GameUrls.JPServerUrls.PatchServerUrl = config.ReadString("redirect.jp", "patch_server", "patch01.psobb.segaonline.jp");
		Settings::GameUrls.JPServerUrls.GameServerUrl = config.ReadString("redirect.jp", "game_server", "game01.psobb.segaonline.jp");
		Settings::GameUrls.EP4ServerUrls.PatchServerUrl = config.ReadString("redirect.ep4", "patch_server", "psobb-ep4-patch.segaonline.jp");
		Settings::GameUrls.EP4ServerUrls.GameServerUrl = config.ReadString("redirect.ep4", "game_server", "psobb-ep4-db.segaonline.jp");
		Settings::GameUrls.CNServerUrls.PatchServerUrl = config.ReadString("redirect.cn", "patch_server", "patch.psobb.cn");
		Settings::GameUrls.CNServerUrls.GameServerUrl = config.ReadString("redirect.cn", "game_server", "db.psobb.cn");
    }

    void Settings::CreateNewConfigFile()
	{
		CIniReader config(CONFIG_FILENAME);

		config.WriteBoolean("patches", "gameguard", false);
		config.WriteBoolean("patches", "redirect", false);
		config.WriteBoolean("patches", "episode4_mode", false);
		config.WriteInteger("patches", "client_language", 1); // AzureFlare::Settings::Language::English
		config.WriteBoolean("patches", "enable_direct_input", false);

        config.WriteString("redirect.us", "patch_server",  "patch01.us.segaonline.jp");
        config.WriteString("redirect.us", "game_server",   "game01.us.segaonline.jp");
        config.WriteString("redirect.jp", "patch_server",  "patch01.psobb.segaonline.jp");
        config.WriteString("redirect.jp", "game_server",   "game01.psobb.segaonline.jp");
        config.WriteString("redirect.cn", "patch_server",  "patch.psobb.cn");
        config.WriteString("redirect.cn", "game_server",   "db.psobb.cn");
        config.WriteString("redirect.ep4", "patch_server", "psobb-ep4-patch.segaonline.jp");
        config.WriteString("redirect.ep4", "game_server",  "psobb-ep4-db.segaonline.jp");
	}
}