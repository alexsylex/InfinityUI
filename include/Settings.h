#pragma once

namespace SKSE::log
{
	using level = spdlog::level::level_enum;
}
namespace logger = SKSE::log;

namespace settings
{
	void Init(const std::string& a_iniFileName);

	// Default values

	namespace debug
	{
		inline logger::level logLevel = logger::level::err;
	}

	namespace patching
	{
		inline bool safeReplacement = true;
		inline std::uint32_t slowPatchWarningMS = 250;
		inline const char* disabledPatchFiles = "";
	}
}
