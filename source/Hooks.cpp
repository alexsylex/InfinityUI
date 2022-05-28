#include "utils/Logger.h"

#include "GFxMoviePatcher.h"

namespace IUI
{
	void PatchGFxMovie(RE::GFxMovieView* a_view, float a_deltaT, std::uint32_t a_frameCatchUpCount)
	{
		std::string_view movieUrl = a_view->GetMovieDef()->GetFileURL();

		logger::trace("Detected GFx movie load from {}", movieUrl);

		a_view->Advance(a_deltaT, a_frameCatchUpCount);

		GFxMoviePatcher moviePatcher(a_view, a_view->GetMovieDef()->GetFileURL());

		if (int loadedSwfPatches = moviePatcher.LoadAvailablePatches()) 
		{
			std::string fmtMessage = "Loaded {} swf patch";
			fmtMessage += loadedSwfPatches > 1 ? "es" : "";
			fmtMessage += " for {}";

			logger::info(fmtMessage, loadedSwfPatches, moviePatcher.GetMovieFilename());
			logger::flush();
		}
	}
}
