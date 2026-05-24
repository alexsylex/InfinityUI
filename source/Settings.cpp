#include "Settings.h"

#include "utils/INISettingCollection.h"

namespace settings
{
	using namespace utils;

	void Init(const std::string& a_iniFileName)
	{
		INISettingCollection* iniSettingCollection = INISettingCollection::GetSingleton();

		{
			using namespace debug;
			iniSettingCollection->AddSettings(
				MakeSetting("uLogLevel:Debug", static_cast<std::uint32_t>(logLevel)));
		}
		{
			using namespace patching;
			iniSettingCollection->AddSettings(
				MakeSetting("bSafeReplacement:Patching", safeReplacement),
				MakeSetting("uSlowPatchWarningMS:Patching", slowPatchWarningMS),
				MakeSetting("sDisabledPatchFiles:Patching", disabledPatchFiles));
		}

		if (!iniSettingCollection->ReadFromFile(a_iniFileName)) {
			logger::warn("Could not read {}, falling back to default options", a_iniFileName);
		}

		{
			using namespace debug;
			logLevel = static_cast<logger::level>(iniSettingCollection->GetSetting<std::uint32_t>("uLogLevel:Debug"));
		}
		{
			using namespace patching;
			safeReplacement = iniSettingCollection->GetSetting<bool>("bSafeReplacement:Patching");
			slowPatchWarningMS = iniSettingCollection->GetSetting<std::uint32_t>("uSlowPatchWarningMS:Patching");
			disabledPatchFiles = iniSettingCollection->GetSetting<const char*>("sDisabledPatchFiles:Patching");
		}
	}
}
