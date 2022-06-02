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

					std::string parentPath = GetMemberParentPath(movieFile);
					if (!parentPath.empty()) 
					{
						RE::GFxValue parentValue;
						if (movieView->GetVariable(&parentValue, parentPath.c_str()))
						{
							if (parentValue.IsDisplayObject()) 
							{
								GFxDisplayObject parent = parentValue;

								std::string memberPath = GetMemberPath(movieFile);
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

								GFxMemberVisitor memberVisitor;

								memberVisitor.VisitMembersOf(parent);
							}
							else 
							{
								AbortReplaceMemberWith(parentValue, movieFile);
							}
						}
					}
				}
			}
		}

		return loadCount;
	}

	void GFxMoviePatcher::CreateMemberFrom(GFxDisplayObject& a_parent, const std::string& a_movieFile)
	{
		std::string memberName = GetMemberName(a_movieFile);

		GFxDisplayObject newDisplayObject = a_parent.CreateEmptyMovieClip(memberName);
		newDisplayObject.LoadMovie(a_movieFile);

		// Actions after loading the movieclip
		API::DispatchMessage(API::PostLoadMessage{ newDisplayObject });
	}

	void GFxMoviePatcher::ReplaceMemberWith(GFxDisplayObject& a_originalMember, GFxDisplayObject& a_parent, const std::string& a_movieFile)
	{
		std::string memberName = GetMemberName(a_movieFile);

		// Last chance to retrieve info before removing the movieclip
		API::DispatchMessage(API::PreLoadMessage{ a_originalMember });

		a_originalMember.RemoveMovieClip();

		CreateMemberFrom(a_parent, a_movieFile);
	}

	void GFxMoviePatcher::AbortReplaceMemberWith(RE::GFxValue& a_originalMember, const std::string& a_movieFile)
	{
		logger::warn("{} exists in the movie, but it is not a DisplayObject. Aborting replacement for {}", 
					 a_originalMember.ToString(), a_movieFile);

		API::DispatchMessage(API::AbortLoadMessage{ a_originalMember });
	}
}
