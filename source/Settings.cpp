#include "Settings.h"

#include "utils/INISettingCollection.h"

namespace settings
{
	struct DefaultCollection : Collection
	{
		DefaultCollection() :
			Collection
			{
				.debug
				{
					.logLevel = logger::level::err
				}
			}
		{ }
	};
}

static const settings::DefaultCollection g_defaultSettings;

namespace settings
{
	using namespace utils;

	void Init(const std::string& a_iniFileName)
	{
		INISettingCollection* iniSettingCollection = INISettingCollection::GetSingleton();

		iniSettingCollection->AddSettings
		(
			// Debug
			MakeSetting("uLogLevel:Debug", static_cast<std::uint32_t>(g_defaultSettings.debug.logLevel))
		);

		if (!iniSettingCollection->ReadFromFile(a_iniFileName))
		{
			logger::warn("Could not read {}, falling back to default options", a_iniFileName);
		}

		// Debug
		g_settings.debug.logLevel = static_cast<logger::level>(iniSettingCollection->GetSetting<std::uint32_t>("uLogLevel:Debug"));
	}
}