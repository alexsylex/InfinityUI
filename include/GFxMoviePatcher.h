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

		void CreateMemberFrom(GFxDisplayObject& a_parent, const std::string& a_movieFile);

		void ReplaceMemberWith(GFxDisplayObject& a_originalMember, GFxDisplayObject& a_parent, const std::string& a_movieFile);

		void AbortReplaceMemberWith(RE::GFxValue& a_originalMember, const std::string& a_movieFile);

		std::string GetMemberPath(const std::string& a_movieFile)
		{
			std::string memberPath;

			std::size_t movieFilenameEnd = a_movieFile.rfind(".swf");
			if (movieFilenameEnd != std::string::npos) 
			{
				std::size_t movieFilenameStart = a_movieFile.find("\\") + 1;
				std::size_t movieFilenameLen = movieFilenameEnd - movieFilenameStart;

				memberPath = a_movieFile.substr(movieFilenameStart, movieFilenameLen);

				std::replace(memberPath.begin(), memberPath.end(), '\\', '.');
			}

			return memberPath;
		}

		std::string GetMemberName(const std::string& a_movieFile)
		{
			std::string memberName;

			std::size_t movieFilenameEnd = a_movieFile.rfind(".swf");
			if (movieFilenameEnd != std::string::npos) 
			{
				std::size_t backslashPos = a_movieFile.rfind("\\");

				std::size_t movieFilenameStart = backslashPos != std::string::npos ? backslashPos + 1 : 0;
				std::size_t movieFilenameLen = movieFilenameEnd - movieFilenameStart;

				memberName = a_movieFile.substr(movieFilenameStart, movieFilenameLen);
			}

			return memberName;
		}

		std::string GetMemberParentPath(const std::string& a_movieFile)
		{
			std::string memberPath = GetMemberPath(a_movieFile);

			std::size_t dotPos = memberPath.rfind(".");

			return dotPos != std::string::npos ? memberPath.substr(0, dotPos) : "_root";
		}

		RE::GFxMovieView* movieView;
		const std::string_view movieDir;
		const std::string_view movieFilename;
		GFxDisplayObject _root{ movieView, "_root" };
		int loadCount = 0;
	};
}
