#include "Hooks.h"

#include "GFxMoviePatcher.h"

namespace IUI
{
	void PatchGFxMovie(RE::GFxMovieView* a_movieView, float a_deltaT, std::uint32_t a_frameCatchUpCount)
	{
		a_movieView->Advance(a_deltaT, a_frameCatchUpCount);

		GFxMoviePatcher(a_movieView).LoadAvailablePatches();
	}

	void InitExtensionsAndNotify(RE::GFxMovieView* a_movieView, const char* a_methodName, const char* a_argFmt, va_list args)
	{
		GFxMovieView::InvokeArgs(a_movieView, a_methodName, a_argFmt, args);

		a_movieView->Advance(0.0F);

		IUI::API::DispatchMessage(IUI::API::PostInitExtensionsMessage{ a_movieView, a_movieView->GetMovieDef()->GetFileURL() });
	}
}
