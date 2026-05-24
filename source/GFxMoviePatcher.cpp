#include "GFxMoviePatcher.h"

#include "GFxDisplayObject.h"
#include "Settings.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <sstream>

namespace IUI
{
	//std::unordered_map<RE::GFxMovieRoot*, GFxMoviePatcher> g_moviePatchersList;

	namespace
	{
		using Clock = std::chrono::steady_clock;

		std::string NormalizePatchPath(std::string a_path)
		{
			std::replace(a_path.begin(), a_path.end(), '/', '\\');
			std::transform(a_path.begin(), a_path.end(), a_path.begin(), [](unsigned char a_char) {
				return static_cast<char>(std::tolower(a_char));
			});
			return a_path;
		}

		void Trim(std::string& a_text)
		{
			const auto start = a_text.find_first_not_of(" \t");
			if (start == std::string::npos) {
				a_text.clear();
				return;
			}

			const auto end = a_text.find_last_not_of(" \t");
			a_text = a_text.substr(start, end - start + 1);
		}

		std::uint64_t ElapsedMS(const Clock::time_point& a_start)
		{
			return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - a_start).count());
		}
	}

	GFxMoviePatcher::GFxMoviePatcher(RE::IMenu* a_menu, RE::GFxMovieRoot* a_movieRoot)
	:	menu{ a_menu }, movieRoot{ a_movieRoot }
	{
		const std::string_view movieRootFileDir = movieRootFileUrl.substr(0, movieRootFileUrl.rfind('/') + 1);
		if (movieRootFileDir.find("Interface/Exported/") != std::string_view::npos)
		{
			movieRootFilePath.append("Exported");
		}
		logger::trace("Detected {} movie load from: {}", movieRootFileBasename, movieRootFilePath.string());
	}

	int GFxMoviePatcher::LoadInstancePatches()
	{
		int loadCount = 0;

		if (std::filesystem::exists(startPath))
		{
			logger::debug("The path \"{}\" exists, loading instance patches...", startPath.string());

			// Actions before we start loading movieclip patches
			API::DispatchMessage(API::StartLoadInstancesMessage{ menu, movieRoot });

			// Non-recursive Depth-First Search to traverse all nodes
			// Reference: https://en.wikipedia.org/wiki/Depth-first_search
			std::stack<std::filesystem::path> stack;

			stack.push(startPath);

			while (!stack.empty())
			{
				std::filesystem::path currentPath = stack.top();
				stack.pop();

				if (std::filesystem::is_directory(currentPath))
				{
					for (const std::filesystem::directory_entry& childPath : std::filesystem::directory_iterator{ currentPath })
					{
						stack.push(childPath);
					}
				}
				else if (currentPath.extension() == ".swf")
				{
					std::string patchRelativePath = currentPath.lexically_relative(movieRootFilePath).string();

					logger::debug("Relative path is \"{}\"", patchRelativePath);

					if (IsPatchDisabled(patchRelativePath)) {
						logger::warn("Skipping disabled InfinityUI patch \"{}\"", patchRelativePath);
						continue;
					}

					const auto patchStart = Clock::now();

					std::string instancePath = GetInstanceASPath(startPath, currentPath);

					std::string parentPath = GetInstanceParentASPath(instancePath);
					if (!parentPath.empty())
					{
						RE::GFxValue member;
						if (movieRoot->GetVariable(&member, parentPath.c_str()))
						{
							if (member.IsDisplayObject())
							{
								GFxDisplayObject parent = member;

								if (!instancePath.empty())
								{
									std::string currentPathStr = currentPath.string();

									logger::debug("Patch found at \"{}\"", currentPathStr);

									std::string instanceName = GetInstanceASName(instancePath);

									if (movieRoot->GetVariable(&member, instancePath.c_str()))
									{
										if (member.IsDisplayObject())
										{
											GFxDisplayObject instance = member;
											if (ReplaceInstance(instanceName, instance, parent, patchRelativePath)) {
												loadCount++;
											}
										}
										else
										{
											AbortReplaceInstance(instanceName, member, patchRelativePath);
										}
									}
									else
									{
										if (AddInstance(instanceName, parent, patchRelativePath)) {
											loadCount++;
										}
									}
								}
							}
							else
							{
								std::string parentName = GetInstanceASName(parentPath);

								AbortReplaceInstance(parentName, member, patchRelativePath);
							}
						}
					}

					const auto patchElapsedMS = ElapsedMS(patchStart);
					if (settings::patching::slowPatchWarningMS > 0 && patchElapsedMS >= settings::patching::slowPatchWarningMS) {
						logger::warn("InfinityUI patch \"{}\" took {} ms", patchRelativePath, patchElapsedMS);
					}
				}
			}

			// Actions after loading all movieclip instance patches
			API::DispatchMessage(API::FinishLoadInstancesMessage{ menu, movieRoot, loadCount });
		}

		return loadCount;
	}

	bool GFxMoviePatcher::AddInstance(const std::string& a_name, GFxDisplayObject& a_parent, const std::string& a_patchRelativePath)
	{
		GFxDisplayObject newInstance = a_parent.CreateEmptyMovieClip(a_name, a_parent.GetNextHighestDepth());

		logger::trace("Relative path to patch is: {}", a_patchRelativePath);

		if (!LoadInstanceMovie(newInstance, a_patchRelativePath)) {
			newInstance.RemoveMovieClip();
			return false;
		}

		logger::trace("");
		LogMembersOf(a_parent);
		logger::trace("");

		return true;
	}

	bool GFxMoviePatcher::ReplaceInstance(const std::string& a_name, GFxDisplayObject& a_originalInstance,
											GFxDisplayObject& a_parent, const std::string& a_patchRelativePath)
	{
		if (settings::patching::safeReplacement) {
			const std::string tempName = MakeTempInstanceName(a_name);
			GFxDisplayObject newInstance = a_parent.CreateEmptyMovieClip(tempName, a_parent.GetNextHighestDepth());

			logger::trace("Relative path to patch is: {}", a_patchRelativePath);

			if (!LoadInstanceMovie(newInstance, a_patchRelativePath, false)) {
				logger::warn("Keeping original {} because replacement patch failed to load from {}", a_name, a_patchRelativePath);
				newInstance.RemoveMovieClip();
				AbortReplaceInstance(a_name, a_originalInstance, a_patchRelativePath);
				return false;
			}

			// Last chance to retrieve info before removing the movieclip
			API::DispatchMessage(API::PreReplaceInstanceMessage{ menu, movieRoot, a_originalInstance });

			logger::trace("");
			LogMembersOf(a_parent);
			LogMembersOf(a_originalInstance);
			logger::trace("");

			a_originalInstance.SwapDepths(1);
			movieRoot->Advance(0);

			a_originalInstance.RemoveMovieClip();
			movieRoot->Advance(0);

			newInstance.SetName(a_name);
			movieRoot->Advance(0);

			auto [movieDefImpl, spriteDef] = newInstance.GetDefs();
			API::DispatchMessage(API::PostPatchInstanceMessage{ menu, movieRoot, newInstance, movieDefImpl, spriteDef });

			logger::trace("Original instance replaced!");
			logger::trace("");
			LogMembersOf(a_parent);
			LogMembersOf(newInstance);
			logger::trace("");

			return true;
		}

		// Last chance to retrieve info before removing the movieclip
		API::DispatchMessage(API::PreReplaceInstanceMessage{ menu, movieRoot, a_originalInstance });

		logger::trace("");
		LogMembersOf(a_parent);
		LogMembersOf(a_originalInstance);
		logger::trace("");

		// MovieClip.removeMovieClip() does not remove a movie clip assigned
		// to a negative depth value. Movie clips created in the authoring tool
		// are assigned negative depth values by default. To remove a movie clip
		// that is assigned to a negative depth value, first use the MovieClip.swapDepths()
		// method to move the movie clip to a positive depth value.
		// Reference: http://homepage.divms.uiowa.edu/~slonnegr/flash/ActionScript2Reference.pdf#page=917
		a_originalInstance.SwapDepths(1);
		movieRoot->Advance(0);

		a_originalInstance.RemoveMovieClip();
		movieRoot->Advance(0);

		logger::trace("Original instance removed!");
		logger::trace("");
		LogMembersOf(a_parent);
		logger::trace("");

		return AddInstance(a_name, a_parent, a_patchRelativePath);
	}

	void GFxMoviePatcher::AbortReplaceInstance(const std::string& a_name, RE::GFxValue& a_instance,
											   const std::string& a_patchRelativePath) const
	{
		logger::warn("{} exists in the movie, but it is not a DisplayObject. Aborting replacement for {}",
					 a_name, a_patchRelativePath);

		API::DispatchMessage(API::AbortPatchInstanceMessage{ menu, movieRoot, a_instance });
	}

	bool GFxMoviePatcher::IsPatchDisabled(const std::string& a_patchRelativePath) const
	{
		const char* disabledPatchFiles = settings::patching::disabledPatchFiles;
		if (!disabledPatchFiles || !disabledPatchFiles[0]) {
			return false;
		}

		const std::string patchPath = NormalizePatchPath(a_patchRelativePath);
		std::stringstream disabledList{ disabledPatchFiles };
		std::string disabledPath;

		while (std::getline(disabledList, disabledPath, ';')) {
			std::stringstream commaList{ disabledPath };
			std::string token;

			while (std::getline(commaList, token, ',')) {
				token = NormalizePatchPath(token);
				Trim(token);

				if (!token.empty() && patchPath.ends_with(token)) {
					return true;
				}
			}
		}

		return false;
	}

	bool GFxMoviePatcher::LoadInstanceMovie(GFxDisplayObject& a_instance, const std::string& a_patchRelativePath, bool a_dispatchPostPatch)
	{
		const auto loadStart = Clock::now();

		if (!a_instance.LoadMovie(a_patchRelativePath)) {
			logger::warn("loadMovie invocation failed for {}", a_patchRelativePath);
			return false;
		}

		movieRoot->Advance(0);

		const auto loadElapsedMS = ElapsedMS(loadStart);
		if (settings::patching::slowPatchWarningMS > 0 && loadElapsedMS >= settings::patching::slowPatchWarningMS) {
			logger::warn("loadMovie for \"{}\" took {} ms", a_patchRelativePath, loadElapsedMS);
		}

		auto [movieDefImpl, spriteDef] = a_instance.GetDefs();
		if (!movieDefImpl || !spriteDef) {
			logger::warn("Could not resolve loaded movie definitions for {}", a_patchRelativePath);
			return false;
		}

		logger::trace("New instance loaded!");
		logger::trace("");
		LogMembersOf(a_instance);
		logger::trace("");

		addedInstanceMovieDefs.push_back(movieDefImpl);

		if (a_dispatchPostPatch) {
			// Actions after loading the movieclip
			API::DispatchMessage(API::PostPatchInstanceMessage{ menu, movieRoot, a_instance, movieDefImpl, spriteDef });
		}

		return true;
	}

	std::string GFxMoviePatcher::MakeTempInstanceName(const std::string& a_name) const
	{
		return "__IUI_LoadCheck_" + a_name;
	}
}
