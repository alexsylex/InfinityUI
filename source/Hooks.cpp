#include "Hooks.h"

#include "GFxMoviePatcher.h"

namespace hooks
{
	void PatchGFxMovie(RE::GFxMovieView* a_movieView, float a_deltaT, std::uint32_t a_frameCatchUpCount)
	{
		a_movieView->Advance(a_deltaT, a_frameCatchUpCount);

		IUI::GFxMoviePatcher(a_movieView).LoadAvailablePatches();
	}

	void InitExtensionsAndNotify(RE::GFxMovieView* a_movieView, const char* a_methodName, const char* a_argFmt, va_list args)
	{
		GFxMovieView::InvokeArgs(a_movieView, a_methodName, a_argFmt, args);

		IUI::API::DispatchMessage(IUI::API::PostInitExtensionsMessage{ a_movieView, a_movieView->GetMovieDef()->GetFileURL() });
	}
}
