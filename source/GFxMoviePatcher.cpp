#include "GFxMoviePatcher.h"

#include "utils/GFxVisitors.h"

#include "utils/Logger.h"

namespace IUI
{
	int GFxMoviePatcher::LoadAvailablePatches()
	{
		std::filesystem::path rootPath = std::filesystem::current_path().append("Data\\Interface");
		if (movieDir.find("Interface/Exported/") != std::string_view::npos) 
		{
			rootPath.append("Exported");
		}

		std::filesystem::path startPath = rootPath;
		startPath.append(GetMovieBasename());

		if (std::filesystem::exists(startPath))
		{
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

					std::string memberToReplacePath = GetMemberToReplacePath(movieFile);

					if (!memberToReplacePath.empty()) 
					{
						logger::debug("{}", currentPath.string().c_str());
						logger::flush();

						RE::GFxValue member;
						if (movieView->GetVariable(&member, memberToReplacePath.c_str())) 
						{
							if (member.IsDisplayObject())
							{
								ReplaceMemberWith(reinterpret_cast<GFxDisplayObject&>(member), movieFile);

								loadCount++;
							} 
							else
							{
								AbortReplaceMemberWith(member, movieFile);
							}
						}
						else 
						{
							CreateMemberFrom(movieFile);
							loadCount++;
						}
					}
				}
			}
		}

		return loadCount;
	}

	void GFxMoviePatcher::CreateMemberFrom(const std::string& a_movieFile)
	{
		GFxDisplayObject patch = _root.CreateEmptyMovieClip(std::string("Patch") + std::to_string(loadCount));
		patch.LoadMovie(a_movieFile);

		API::DispatchMessage(API::PostLoadMessage{ "create" });
	}

	void GFxMoviePatcher::ReplaceMemberWith(const GFxDisplayObject& a_originalMember, const std::string& a_movieFile)
	{
		API::DispatchMessage(API::PreLoadMessage{ &a_originalMember });

		GFxDisplayObject patch = _root.CreateEmptyMovieClip(std::string("Patch") + std::to_string(loadCount));
		patch.LoadMovie(a_movieFile);

		API::DispatchMessage(API::PostLoadMessage{ "replace" });
	}

	void GFxMoviePatcher::AbortReplaceMemberWith(const RE::GFxValue& a_originalMember, const std::string& a_movieFile)
	{
		logger::warn("{} exists in the movie, but it is not a DisplayObject. Aborting replacement for {}", 
					 a_originalMember.ToString(), a_movieFile);

		API::DispatchMessage(API::AbortLoadMessage{ &a_originalMember });
	}
}
