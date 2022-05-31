#pragma once

#include "FullAPI.h"

namespace IUI
{
	class GFxMoviePatcher
	{
	public:

		GFxMoviePatcher(RE::GFxMovieView* a_movieView, const std::string_view& a_movieUrl)
		: movieView{ a_movieView }, movieDir{ a_movieUrl.substr(0, a_movieUrl.rfind('/') + 1) },
		  movieFilename{ a_movieUrl.substr(a_movieUrl.rfind('/') + 1) }
		{ }

		std::string_view GetMovieBasename() const { return movieFilename.substr(0, movieFilename.find('.')); }

		int LoadAvailablePatches();

	private:

		void CreateMemberFrom(const std::string& a_movieFile);
		void ReplaceMemberWith(const GFxDisplayObject& a_originalMember, const std::string& a_movieFile);
		void AbortReplaceMemberWith(const RE::GFxValue& a_originalMember, const std::string& a_movieFile);

		std::string GetMemberToReplacePath(const std::string& a_movieFile)
		{
			std::string memberToReplacePath;

			std::size_t movieFilenameEnd = a_movieFile.find(".swf", a_movieFile.size() - 4);
			if (movieFilenameEnd != std::string::npos) 
			{
				std::size_t movieFilenameStart = a_movieFile.find("\\") + 1;
				std::size_t movieFilenameLen = movieFilenameEnd - movieFilenameStart;

				memberToReplacePath = a_movieFile.substr(movieFilenameStart, movieFilenameLen);

				std::replace(memberToReplacePath.begin(), memberToReplacePath.end(), '\\', '.');
			}

			return memberToReplacePath;
		}

		RE::GFxMovieView* movieView;
		const std::string_view movieDir;
		const std::string_view movieFilename;
		GFxDisplayObject _root{ movieView, "_root" };
		int loadCount = 0;
	};
}
