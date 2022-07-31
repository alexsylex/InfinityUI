#include "GFxMoviePatcher.h"

#include "utils/GFxLoggers.h"

namespace IUI
{
	GFxMoviePatcher::GFxMoviePatcher(RE::GFxMovieView* a_movieView)
	:	movieView{ a_movieView }
	{
		logger::debug("Detected GFx movie load from {}", movieViewUrl);
	}

	void GFxMoviePatcher::LoadAvailablePatches() const&&
	{
		int loadCount = 0;

		std::filesystem::path movieViewPath = std::filesystem::current_path().append("Data\\Interface");
		if (movieViewDir.find("Interface/Exported/") != std::string_view::npos)
		{
			movieViewPath.append("Exported");
		}

		if (std::filesystem::exists(startPath))
		{
			logger::debug("The path \"{}\" exists, loading existing patches...", startPath.string().c_str());

			// Actions before we start loading movieclip patches
			API::DispatchMessage(API::StartLoadMessage{ movieView, movieViewUrl });

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
					std::string patchRelativePath = std::filesystem::relative(currentPath, movieViewPath).string().c_str();

					std::string memberPath = GetPatchedMemberPath(currentPath);

					std::string parentPath = GetPatchedMemberParentPath(memberPath);
					if (!parentPath.empty())
					{
						RE::GFxValue parentValue;
						if (movieView->GetVariable(&parentValue, parentPath.c_str()))
						{
							if (parentValue.IsDisplayObject()) 
							{
								GFxDisplayObject parent = parentValue;

								if (!memberPath.empty())
								{
									logger::debug("Patch found at \"{}\"", currentPath.string().c_str());

									std::string memberName = GetPatchedMemberName(memberPath);

									RE::GFxValue memberValue;
									if (movieView->GetVariable(&memberValue, memberPath.c_str()))
									{
										if (memberValue.IsDisplayObject())
										{
											GFxDisplayObject member = memberValue;

											ReplaceMemberWith(memberName, member, parent, patchRelativePath);

											loadCount++;
										}
										else 
										{
											AbortReplaceMemberWith(memberValue, patchRelativePath);
										}
									}
									else 
									{
										CreateMemberFrom(memberName, parent, patchRelativePath);
										loadCount++;
									}
								}
							}
							else 
							{
								AbortReplaceMemberWith(parentValue, patchRelativePath);
							}
						}
					}
				}
			}

			// Actions after loading all movieclip patches
			API::DispatchMessage(API::FinishLoadMessage{ movieView, movieViewUrl, loadCount });
		}

		if (loadCount) 
		{
			logger::info("Patches loaded for {}: {}", movieViewUrl, loadCount);
		}
		else
		{
			logger::debug("Patches loaded for {}: 0", movieViewUrl);
		}
		logger::debug("");
	}

	void GFxMoviePatcher::CreateMemberFrom(const std::string_view& a_memberName, GFxDisplayObject& a_parent, const std::string& a_patchRelativePath) const
	{
		GFxMemberLogger<logger::level::trace> memberLogger;

		GFxDisplayObject newDisplayObject = a_parent.CreateEmptyMovieClip(a_memberName, a_parent.GetNextHighestDepth());
		newDisplayObject.LoadMovie(a_patchRelativePath);

		logger::trace("After loading MovieClip");
		logger::trace("");
		memberLogger.LogMembersOf(a_parent);
		memberLogger.LogMembersOf(newDisplayObject);
		logger::trace("");

		// Actions after loading the movieclip
		API::DispatchMessage(API::PostPatchMessage{ movieView, movieViewUrl, newDisplayObject });
	}

	void GFxMoviePatcher::ReplaceMemberWith(const std::string_view& a_memberName, GFxDisplayObject& a_originalMember,
											GFxDisplayObject& a_parent, const std::string& a_patchRelativePath) const
	{
		GFxMemberLogger<logger::level::trace> memberLogger;

		// Last chance to retrieve info before removing the movieclip
		API::DispatchMessage(API::PreReplaceMessage{ movieView, movieViewUrl, a_originalMember });

		logger::trace("Before removing MovieClip");
		logger::trace("");
		memberLogger.LogMembersOf(a_parent);
		memberLogger.LogMembersOf(a_originalMember);
		logger::trace("");

		// MovieClip.removeMovieClip() does not remove a movie clip assigned
		// to a negative depth value. Movie clips created in the authoring tool
		// are assigned negative depth values by default. To remove a movie clip
		// that is assigned to a negative depth value, first use the MovieClip.swapDepths()
		// method to move the movie clip to a positive depth value.
		// Reference: http://homepage.divms.uiowa.edu/~slonnegr/flash/ActionScript2Reference.pdf#page=917
		a_originalMember.SwapDepths(1);
		a_originalMember.RemoveMovieClip();

		logger::trace("After removing MovieClip");
		logger::trace("");
		memberLogger.LogMembersOf(a_parent);
		logger::trace("");

		CreateMemberFrom(a_memberName, a_parent, a_patchRelativePath);
	}

	void GFxMoviePatcher::AbortReplaceMemberWith(RE::GFxValue& a_originalMember, const std::string& a_patchRelativePath) const
	{
		logger::warn("{} exists in the movie, but it is not a DisplayObject. Aborting replacement for {}", 
					 a_originalMember.ToString(), a_patchRelativePath);

		API::DispatchMessage(API::AbortPatchMessage{ movieView, movieViewUrl, a_originalMember });
	}
}
