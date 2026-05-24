#include "RE/B/BStimer.h"

#include "Hooks.h"

#include "GFxMoviePatcher.h"

#include <unordered_set>

namespace hooks
{
	void PatchGFxMovie(RE::GFxMovieRoot* a_movieRoot, float a_deltaT, std::uint32_t a_frameCatchUpCount, RE::IMenu* a_menu)
	{
		a_movieRoot->Advance(a_deltaT, a_frameCatchUpCount);

		static thread_local std::unordered_set<RE::GFxMovieRoot*> patchingMovieRoots;
		const auto [it, inserted] = patchingMovieRoots.insert(a_movieRoot);
		if (!inserted) {
			logger::warn("Skipping recursive InfinityUI patch pass for {}", a_movieRoot->GetMovieDef()->GetFileURL());
			return;
		}

		IUI::GFxMoviePatcher moviePatcher{ a_menu, a_movieRoot };

		int loadedInstancesCount = moviePatcher.LoadInstancePatches();

		patchingMovieRoots.erase(it);


		const char* movieRootFileUrl = a_movieRoot->GetMovieDef()->GetFileURL();

		if (loadedInstancesCount) 
		{
			logger::info("Patches loaded for {}: {}", movieRootFileUrl, loadedInstancesCount);
		}
		else
		{
			logger::debug("Patches loaded for {}: 0", movieRootFileUrl);
		}
		logger::debug("");

		//if (loadedInstancesCount)
		//{
		//	IUI::g_moviePatchersList.emplace(a_movieRoot, moviePatcher);
		//}
	}

	void InitExtensionsAndNotify(RE::GFxMovieRoot* a_movieRoot, const char* a_methodName, const char* a_argFmt, RE::IMenu* a_menu, va_list args)
	{
		GFxMovieRoot::InvokeArgs(a_movieRoot, a_methodName, a_argFmt, args);

		//if (IUI::g_moviePatchersList.contains(a_movieRoot))
		//{
			IUI::API::DispatchMessage(IUI::API::PostInitExtensionsMessage{ a_menu, a_movieRoot });
		//}
	}
}
