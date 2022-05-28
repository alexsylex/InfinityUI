#pragma once

#include "utils/Trampoline.h"

#include "RE/B/BSCoreTypes.h"

namespace IUI
{
	class BSScaleformManager
	{
#if BUILD_SE
		static constinit inline REL::ID LoadMovieId{ 80302 };
#else
		static constinit inline REL::ID LoadMovieId{ 0 };
#endif
	public:

		static inline REL::Relocation<bool (*)(const RE::BSScaleformManager*, RE::IMenu*,
			RE::GPtr<RE::GFxMovieView>&, const char*, RE::GFxMovieView::ScaleModeType, float)>
			LoadMovie{ LoadMovieId };
	};

	void PatchGFxMovie(RE::GFxMovieView* a_viewOut, float a_deltaT, std::uint32_t a_frameCatchUpCount);

	static inline void InstallHooks()
	{
		// BSScaleformManager::LoadMovie
		{
			static std::uintptr_t hookedAddress = BSScaleformManager::LoadMovie.address() + 0x39B;

			struct HookCode : Xbyak::CodeGenerator
			{
				HookCode()
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					call(ptr[rip + hookLabel]);

					jmp(ptr[rip + retnLabel]);

					L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&PatchGFxMovie));
					L(retnLabel), dq(hookedAddress + 6);
				}
			};

			utils::WriteBranchTrampoline<6>(hookedAddress, HookCode());
		}
	}
}
