#include "GFxMoviePatcher.h"

#include "RE/G/GFxMovieView.h"

#include "utils/GFxVisitors.h"

#include "utils/Logger.h"

namespace IUI
{
	int GFxMoviePatcher::LoadAvailablePatches()
	{
		namespace fs = std::filesystem;

		fs::path rootPath = fs::current_path().append("Data\\Interface");
		if (movieDir.find("Interface/Exported/") != std::string_view::npos) 
		{
			rootPath.append("Exported");
		}

		fs::path startPath = rootPath;
		startPath.append(GetMovieFilename());
		
		int loadedPatchesCount = 0;

		if (fs::exists(startPath))
		{
			// Non-recursive Depth-First Search to traverse all nodes
			// Reference: https://en.wikipedia.org/wiki/Depth-first_search
			std::stack<fs::path> stack;

			stack.push(startPath);

			while (!stack.empty())
			{
				fs::path currentPath = stack.top();
				stack.pop();
		
				if (fs::is_directory(currentPath))
				{
					for (const fs::directory_entry& childPath : fs::directory_iterator{ currentPath }) 
					{
						stack.push(childPath);
					}
				}
				else
				{
					std::string filePath = fs::relative(currentPath, rootPath).string().c_str();

					std::size_t filePathEnd = filePath.find(".swf", filePath.size() - 4);
					if (filePathEnd != std::string::npos)
					{
						logger::debug("{}", currentPath.string().c_str());
						logger::flush();

						std::size_t filePathStart = filePath.find("\\") + 1;
						std::size_t filePathLen = filePathEnd - filePathStart;

						std::string fullQualyMemberName = filePath.substr(filePathStart, filePathLen);

						std::replace(fullQualyMemberName.begin(), fullQualyMemberName.end(), '\\', '.');

						RE::GFxValue member;
						if (movieView->GetVariable(&member, fullQualyMemberName.c_str()))
						{
							std::string patchName = "Patch";
							patchName += std::to_string(loadedPatchesCount);

							GFxMovieClip patch = _root.CreateEmptyMovieClip(patchName);
							patch.LoadMovie(filePath);

							char message[] = "test";

							SKSE::GetMessagingInterface()->Dispatch(0, message, (uint32_t)std::strlen(message), nullptr);

							loadedPatchesCount++;
						}
					}
				}
			}
		}

		return loadedPatchesCount;
	}
}
