#pragma once

namespace SKSE::log
{
	using level = spdlog::level::level_enum;
}
namespace logger = SKSE::log;

namespace settings
{
	struct Collection
	{
		struct Debug
		{
			logger::level logLevel;
		};

		Debug debug;
	};

	void Init(const std::string& a_iniFileName);
}

inline settings::Collection g_settings;