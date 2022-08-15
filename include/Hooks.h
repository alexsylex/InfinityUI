#pragma once

#include "utils/Trampoline.h"

#include "RE/B/BSCoreTypes.h"

namespace hooks
{
	class BSScaleformManager
	{
		static constexpr REL::RelocationID LoadMovieId = RELOCATION_ID(80302, 82325);

	public:

		static inline REL::Relocation<bool (*)(const RE::BSScaleformManager*, RE::IMenu*,
			RE::GPtr<RE::GFxMovieView>&, const char*, RE::GFxMovieView::ScaleModeType, float)>
			LoadMovie{ LoadMovieId };
	};

	class GFxMovieView
	{
	public:

		static inline REL::Relocation<const char* (*)(RE::GFxMovieView*, const char*, const char*, va_list)>
			InvokeArgs;
	};

	void PatchGFxMovie(RE::GFxMovieView* a_movieView, float a_deltaT, std::uint32_t a_frameCatchUpCount);
	void InitExtensionsAndNotify(RE::GFxMovieView* a_movieView, const char* a_methodName, const char* a_argFmt, va_list args);

	static inline void Install()
	{
		// BSScaleformManager::LoadMovie call to movieView->Advance(0.0F, 2)
		{
			static std::uint64_t hookedFunctionOffset = REL::Module::IsSE() ? 0x39B : 0x399;
			static std::uintptr_t hookedAddress = BSScaleformManager::LoadMovie.address() + hookedFunctionOffset;

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

		// BSScaleformManager::LoadMovie call to movieView->InvokeArgs("_root.InitExtensions", nullptr, nullptr)
		{
			static std::uint64_t hookedFunctionOffset = REL::Module::IsSE() ? 0x3C2 : 0x3C0;
			static std::uintptr_t hookedAddress = BSScaleformManager::LoadMovie.address() + hookedFunctionOffset;

			struct HookCode : Xbyak::CodeGenerator
			{
				HookCode()
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					call(ptr[rip + hookLabel]);

					jmp(ptr[rip + retnLabel]);

					L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&InitExtensionsAndNotify));
					L(retnLabel), dq(hookedAddress + 5);
				}
			};

			GFxMovieView::InvokeArgs = utils::WriteBranchTrampoline<5>(hookedAddress, HookCode());
		}
	}
}
