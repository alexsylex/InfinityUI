#pragma once

#include "FullAPI.h"

namespace IUI
{
	class GFxMoviePatcher
	{
	public:

		GFxMoviePatcher(RE::GFxMovieView* a_movieView);

		void LoadAvailablePatches() const&&;

	private:

		std::string_view GetContextMovieUrl() const { return movieView->GetMovieDef()->GetFileURL(); }

		std::string_view GetContextMovieDir() const
		{
			std::string_view movieUrl = GetContextMovieUrl();
			return movieUrl.substr(0, movieUrl.rfind('/') + 1);
		}

		std::string_view GetContextMovieFileName() const
		{
			std::string_view movieUrl = GetContextMovieUrl();
			return movieUrl.substr(movieUrl.rfind('/') + 1);
		}

		std::string_view GetContextMovieBasename() const
		{
			std::string_view movieFilename = GetContextMovieFileName();
			return movieFilename.substr(0, movieFilename.find('.'));
		}

		std::string GetPatchedMemberPath(const std::string& a_movieFile) const
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

		std::string GetPatchedMemberName(const std::string& a_movieFile) const
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

		std::string GetPatchedMemberParentPath(const std::string& a_movieFile) const
		{
			std::string memberPath = GetPatchedMemberPath(a_movieFile);

			std::size_t dotPos = memberPath.rfind(".");

			return dotPos != std::string::npos ? memberPath.substr(0, dotPos) : "_root";
		}

		void CreateMemberFrom(GFxDisplayObject& a_parent, const std::string& a_movieFile) const;

		void ReplaceMemberWith(GFxDisplayObject& a_originalMember, GFxDisplayObject& a_parent, const std::string& a_movieFile) const;

		void AbortReplaceMemberWith(RE::GFxValue& a_originalMember, const std::string& a_movieFile) const;

		RE::GFxMovieView* movieView;
	};
}
