#include "GFxMoviePatcher.h"

#include "utils/GFxVisitors.h"

#include "utils/Logger.h"

namespace IUI
{
	GFxMoviePatcher::GFxMoviePatcher(RE::GFxMovieView* a_movieView)
	:	movieView{ a_movieView }
	{
		logger::debug("Detected GFx movie load from {}",GetContextMovieUrl());
		logger::flush();
	}

	void GFxMoviePatcher::LoadAvailablePatches() const&&
	{
		int loadCount = 0;

		std::filesystem::path rootPath = std::filesystem::current_path().append("Data\\Interface");
		if (GetContextMovieDir().find("Interface/Exported/") != std::string_view::npos)
		{
			rootPath.append("Exported");
		}

		std::filesystem::path startPath = rootPath;
		startPath.append(GetContextMovieBasename());

		if (std::filesystem::exists(startPath))
		{
			// Actions before we start loading movieclip patches
			API::DispatchMessage(API::StartLoadMessage{ movieView, GetContextMovieUrl() });

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
				else
				{
					std::string movieFile = std::filesystem::relative(currentPath, rootPath).string().c_str();

					std::string parentPath = GetPatchedMemberParentPath(movieFile);
					if (!parentPath.empty()) 
					{
						RE::GFxValue parentValue;
						if (movieView->GetVariable(&parentValue, parentPath.c_str()))
						{
							if (parentValue.IsDisplayObject()) 
							{
								GFxDisplayObject parent = parentValue;

								std::string memberPath = GetPatchedMemberPath(movieFile);
								if (!memberPath.empty()) 
								{
									logger::debug("{}", currentPath.string().c_str());
									logger::flush();

									RE::GFxValue memberValue;
									if (movieView->GetVariable(&memberValue, memberPath.c_str()))
									{
										if (memberValue.IsDisplayObject())
										{
											GFxDisplayObject member = memberValue;

											ReplaceMemberWith(member, parent, movieFile);

											loadCount++;
										}
										else 
										{
											AbortReplaceMemberWith(memberValue, movieFile);
										}
									}
									else 
									{
										CreateMemberFrom(parent, movieFile);
										loadCount++;
									}
								}

								//GFxMemberVisitor memberVisitor;
								//memberVisitor.VisitMembersOf(parent);
							}
							else 
							{
								AbortReplaceMemberWith(parentValue, movieFile);
							}
						}
					}
				}
			}

			// Actions after loading all movieclip patches
			API::DispatchMessage(API::FinishLoadMessage{ movieView, GetContextMovieUrl(), loadCount });
		}

		if (loadCount) 
		{
			logger::info("Patches loaded for {}: {}", GetContextMovieUrl(), loadCount);
		}
		else
		{
			logger::debug("Patches loaded for {}: 0", GetContextMovieUrl());
		}
		logger::flush();
	}

	void GFxMoviePatcher::CreateMemberFrom(GFxDisplayObject& a_parent, const std::string& a_movieFile) const
	{
		std::string memberName = GetPatchedMemberName(a_movieFile);

		GFxDisplayObject newDisplayObject = a_parent.CreateEmptyMovieClip(memberName);
		newDisplayObject.LoadMovie(a_movieFile);
		movieView->Advance(0.0F);

		// Actions after loading the movieclip
		API::DispatchMessage(API::PostPatchMessage{ movieView, GetContextMovieUrl(), newDisplayObject });
	}

	void GFxMoviePatcher::ReplaceMemberWith(GFxDisplayObject& a_originalMember, GFxDisplayObject& a_parent, const std::string& a_movieFile) const
	{
		std::string memberName = GetPatchedMemberName(a_movieFile);

		// Last chance to retrieve info before removing the movieclip
		API::DispatchMessage(API::PreReplaceMessage{ movieView, GetContextMovieUrl(), a_originalMember });

		// MovieClip.removeMovieClip() does not remove a movie clip assigned
		// to a negative depth value. Movie clips created in the authoring tool
		// are assigned negative depth values by default. To remove a movie clip
		// that is assigned to a negative depth value, first use the MovieClip.swapDepths()
		// method to move the movie clip to a positive depth value.
		// Reference: http://homepage.divms.uiowa.edu/~slonnegr/flash/ActionScript2Reference.pdf#page=917
		a_originalMember.SwapDepths(1);
		movieView->Advance(0.0F);
		a_originalMember.RemoveMovieClip();
		movieView->Advance(0.0F);

		CreateMemberFrom(a_parent, a_movieFile);
	}

	void GFxMoviePatcher::AbortReplaceMemberWith(RE::GFxValue& a_originalMember, const std::string& a_movieFile) const
	{
		logger::warn("{} exists in the movie, but it is not a DisplayObject. Aborting replacement for {}", 
					 a_originalMember.ToString(), a_movieFile);

		API::DispatchMessage(API::AbortPatchMessage{ movieView, GetContextMovieUrl(), a_originalMember });
	}
}
