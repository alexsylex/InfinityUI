#pragma once

#include "FullAPI.h"

#include "GFxLoggers.h"

namespace IUI
{
	class GFxDisplayObject;

	class GFxMoviePatcher : 
		protected GFxMemberLogger<logger::level::trace>,
		protected GFxArrayLogger<logger::level::trace>
	{
	public:

		GFxMoviePatcher(RE::IMenu* a_menu, RE::GFxMovieRoot* a_movieRoot);

		int LoadInstancePatches();

	private:

		std::filesystem::path GetMovieRootFilePath() const
		{
			std::filesystem::path path = std::filesystem::current_path().append("Data\\Interface");

			const std::string_view movieFileDir = movieRootFileUrl.substr(0, movieRootFileUrl.rfind('/') + 1);

			if (movieFileDir.find("Interface/Exported/") != std::string_view::npos)
			{
				path.append("Exported");
			}

			return path;
		}

		std::string GetInstanceASPath(const std::filesystem::path& a_startPath,
									  const std::filesystem::path& a_movieFilePath) const
		{
			std::string memberPath;

			std::string movieFilePath = a_movieFilePath.lexically_relative(a_startPath).string();

			logger::trace("Relative path to SWF: {}", movieFilePath);

			std::size_t movieFilenameLen = movieFilePath.rfind(".swf");
			if (movieFilenameLen != std::string::npos)
			{
				memberPath = movieFilePath.substr(0, movieFilenameLen);

				std::replace(memberPath.begin(), memberPath.end(), '\\', '.');
			}

			return memberPath;
		}

		std::string GetInstanceASName(const std::string& a_memberPath) const
		{
			std::size_t dotPos = a_memberPath.rfind('.');

			std::size_t memberNameStart = dotPos != std::string::npos ? dotPos + 1 : 0;
			std::size_t memberNameLen = a_memberPath.size() - memberNameStart;

			return a_memberPath.substr(memberNameStart, memberNameLen).data();
		}

		std::string GetInstanceParentASPath(const std::string& a_memberPath) const
		{
			std::size_t dotPos = a_memberPath.rfind(".");

			return std::string(dotPos != std::string::npos ? a_memberPath.substr(0, dotPos) : "_root");
		}

		void AddInstance(const std::string& a_name, GFxDisplayObject& a_parent, const std::string& a_patchRelativePath);

		void ReplaceInstance(const std::string& a_name, GFxDisplayObject& a_originalInstance,
							 GFxDisplayObject& a_parent, const std::string& a_patchRelativePath);

		void AbortReplaceInstance(const std::string& a_name, RE::GFxValue& a_originalInstance, const std::string& a_patchRelativePath) const;

		// members
		RE::IMenu* menu;

		RE::GFxMovieRoot* movieRoot;
		RE::GFxMovieDefImpl* movieRootDef = static_cast<RE::GFxMovieDefImpl*>(movieRoot->GetMovieDef());

		const std::string_view		movieRootFileUrl = movieRootDef->GetFileURL();
		const std::string_view		movieRootFileName = movieRootFileUrl.substr(movieRootFileUrl.rfind('/') + 1);
		const std::string_view		movieRootFileBasename = movieRootFileName.substr(0, movieRootFileName.find('.'));
		std::filesystem::path		movieRootFilePath = std::filesystem::current_path().append("Data\\Interface");
		const std::filesystem::path startPath = std::filesystem::current_path().append("Data\\Interface\\InfinityUI").append(movieRootFileBasename);

		std::vector<RE::GFxMovieDefImpl*> addedInstanceMovieDefs;
	};

	//extern std::unordered_map<RE::GFxMovieRoot*, GFxMoviePatcher> g_moviePatchersList;
}
