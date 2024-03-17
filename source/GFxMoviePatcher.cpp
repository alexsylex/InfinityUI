#include "GFxMoviePatcher.h"

#include "GFxDisplayObject.h"

namespace IUI
{
	//std::unordered_map<RE::GFxMovieRoot*, GFxMoviePatcher> g_moviePatchersList;

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
											ReplaceInstance(instanceName, instance, parent, patchRelativePath);
											loadCount++;
										}
										else
										{
											AbortReplaceInstance(instanceName, member, patchRelativePath);
										}
									}
									else 
									{
										AddInstance(instanceName, parent, patchRelativePath);
										loadCount++;
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
				}
			}

			// Actions after loading all movieclip instance patches
			API::DispatchMessage(API::FinishLoadInstancesMessage{ menu, movieRoot, loadCount });
		}

		return loadCount;
	}

	void GFxMoviePatcher::AddInstance(const std::string& a_name, GFxDisplayObject& a_parent, const std::string& a_patchRelativePath)
	{
		GFxDisplayObject newInstance = a_parent.CreateEmptyMovieClip(a_name, a_parent.GetNextHighestDepth());

		logger::trace("Relative path to patch is: {}", a_patchRelativePath);

		newInstance.LoadMovie(a_patchRelativePath);
		movieRoot->Advance(0);

		logger::trace("New instance loaded!");
		logger::trace("");
		LogMembersOf(a_parent);
		LogMembersOf(newInstance);
		logger::trace("");

		auto [movieDefImpl, spriteDef] = newInstance.GetDefs();

		addedInstanceMovieDefs.push_back(movieDefImpl);

		// Actions after loading the movieclip
		API::DispatchMessage(API::PostPatchInstanceMessage{ menu, movieRoot, newInstance, movieDefImpl, spriteDef });
	}

	void GFxMoviePatcher::ReplaceInstance(const std::string& a_name, GFxDisplayObject& a_originalInstance,
											GFxDisplayObject& a_parent, const std::string& a_patchRelativePath)
	{
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

		AddInstance(a_name, a_parent, a_patchRelativePath);
	}

	void GFxMoviePatcher::AbortReplaceInstance(const std::string& a_name, RE::GFxValue& a_instance,
											   const std::string& a_patchRelativePath) const
	{
		logger::warn("{} exists in the movie, but it is not a DisplayObject. Aborting replacement for {}", 
					 a_name, a_patchRelativePath);

		API::DispatchMessage(API::AbortPatchInstanceMessage{ menu, movieRoot, a_instance });
	}
}
