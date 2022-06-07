#include "GFxMoviePatcher.h"

namespace IUI
{
	void PatchGFxMovie(RE::GFxMovieView* a_movieView, float a_deltaT, std::uint32_t a_frameCatchUpCount)
	{
		a_movieView->Advance(a_deltaT, a_frameCatchUpCount);

		GFxMoviePatcher{ a_movieView }.LoadAvailablePatches();
	}
}
