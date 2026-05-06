#pragma once

namespace AzureFlare
{
    class Settings
    {
    public:
        enum class Language : int
		{
			Japanese,
			English,
			German,
			French,
			Spanish,
			SimplifiedChinese,
			TraditionalChinese,
			Korean
		};

		struct ServerUrls
		{
			std::string PatchServerUrl;
			std::string GameServerUrl;
		};

		struct PsoBBServerUrls
		{
			ServerUrls USAServerUrls;
			ServerUrls JPServerUrls;
			ServerUrls CNServerUrls;
			ServerUrls EP4ServerUrls;
		};

        static void Init();

        static bool DisableGameGuard;
        static bool EnableServerRedirection;
        static Language CurrentLanguage;
        static bool EnableEpisode4Mode;
        static bool DisableIMEInput;
        static PsoBBServerUrls GameUrls;

    private:
		static void LoadConfigurationFile();
		static void CreateNewConfigFile();
    };
}