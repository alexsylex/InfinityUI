#pragma once

#include "FullAPI.h"
#include "utils/GFxDisplayObject.h"

namespace IUI
{
	class GFxMoviePatcher
	{
	public:

		GFxMoviePatcher(RE::GFxMovieView* a_movieView);

		void LoadAvailablePatches() const&&;

	private:

		std::string GetPatchedMemberPath(const std::filesystem::path& a_movieFilePath) const
		{
			std::string memberPath;

			std::string movieFilePath = std::filesystem::relative(a_movieFilePath, startPath).string().c_str();

			std::size_t movieFilenameLen = movieFilePath.rfind(".swf");
			if (movieFilenameLen != std::string::npos) 
			{
				memberPath = movieFilePath.substr(0, movieFilenameLen);

				std::replace(memberPath.begin(), memberPath.end(), '\\', '.');
			}

			return memberPath;
		}

		std::string GetPatchedMemberName(const std::string_view& a_memberPath) const
		{
			std::size_t dotPos = a_memberPath.rfind('.');

			std::size_t memberNameStart = dotPos != std::string::npos ? dotPos + 1 : 0;
			std::size_t memberNameLen = a_memberPath.size() - memberNameStart;

			return a_memberPath.substr(memberNameStart, memberNameLen).data();
		}

		std::string GetPatchedMemberParentPath(const std::string_view& a_memberPath) const
		{
			std::size_t dotPos = a_memberPath.rfind(".");

			return std::string(dotPos != std::string_view::npos ? a_memberPath.substr(0, dotPos) : "_root");
		}

		void CreateMemberFrom(const std::string_view& a_memberName, GFxDisplayObject& a_parent, const std::string& a_patchRelativePath) const;

		void ReplaceMemberWith(const std::string_view& a_memberName, GFxDisplayObject& a_originalMember,
							   GFxDisplayObject& a_parent, const std::string& a_patchRelativePath) const;

		void AbortReplaceMemberWith(RE::GFxValue& a_originalMember, const std::string& a_patchRelativePath) const;

		RE::GFxMovieView* movieView;
		const std::string_view movieViewUrl = movieView->GetMovieDef()->GetFileURL();
		const std::string_view movieViewDir = movieViewUrl.substr(0, movieViewUrl.rfind('/') + 1);
		const std::string_view movieViewFileName = movieViewUrl.substr(movieViewUrl.rfind('/') + 1);
		const std::string_view movieViewBasename = movieViewFileName.substr(0, movieViewFileName.find('.'));

		const std::filesystem::path startPath = std::filesystem::current_path().append("Data\\Interface\\InfinityUI").append(movieViewBasename);
	};
}
