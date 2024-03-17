#pragma once

#include "utils/Trampoline.h"

#include "RE/B/BSCoreTypes.h"

namespace hooks
{
	class BSScaleformManager
	{
		static constexpr REL::RelocationID LoadMovieId{ 80302, 82325 };

	public:

		static inline REL::Relocation<bool(*)(const RE::BSScaleformManager*, RE::IMenu*,
											  RE::GPtr<RE::GFxMovieView>&, const char*,
											  RE::GFxMovieView::ScaleModeType, float)> LoadMovie{ LoadMovieId };
	};

	class GFxMovieRoot
	{
	public:

		static inline REL::Relocation<const char*(*)(RE::GFxMovieRoot*, const char*, const char*, va_list)> InvokeArgs;
	};

	void PatchGFxMovie(RE::GFxMovieRoot* a_movieRoot, float a_deltaT, std::uint32_t a_frameCatchUpCount, RE::IMenu* a_menu);
	void InitExtensionsAndNotify(RE::GFxMovieRoot* a_movieRoot, const char* a_methodName, const char* a_argFmt,
								 RE::IMenu* a_menu, va_list args);

	static inline void Install()
	{
		// BSScaleformManager::LoadMovie call to movieView->Advance(0.0F, 2)
		struct PatchGFxMovieHook : Hook<6>
		{
			struct HookCodeGenerator : Xbyak::CodeGenerator
			{
				HookCodeGenerator(std::uintptr_t a_hookedAddress)
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					mov(r9, rdi);  // rdi = IMenu*
					call(ptr[rip + hookLabel]);

					jmp(ptr[rip + retnLabel]);

					L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&PatchGFxMovie));
					L(retnLabel), dq(a_hookedAddress + 6);

					ready();
				}
			};

			PatchGFxMovieHook(std::uintptr_t a_hookedAddress) :
				Hook<6>{ a_hookedAddress, HookCodeGenerator{ a_hookedAddress } }
			{}
		};

		PatchGFxMovieHook patchGFxMovieHook{ BSScaleformManager::LoadMovie.address() + (REL::Module::IsSE() ? 0x39B : 0x399) };

		// BSScaleformManager::LoadMovie call to movieView->InvokeArgs("_root.InitExtensions", nullptr, nullptr)
		struct InitExtensionsAndNotifyHook : Hook<5>
		{
			struct HookCodeGenerator : Xbyak::CodeGenerator
			{
				HookCodeGenerator(std::uintptr_t a_hookedAddress)
				{
					Xbyak::Label hookLabel;
					Xbyak::Label retnLabel;

					mov(r9, rdi);  // rdi = IMenu*
					call(ptr[rip + hookLabel]);

					jmp(ptr[rip + retnLabel]);

					L(hookLabel), dq(reinterpret_cast<std::uintptr_t>(&InitExtensionsAndNotify));
					L(retnLabel), dq(a_hookedAddress + 5);

					ready();
				}
			};

			InitExtensionsAndNotifyHook(std::uintptr_t a_hookedAddress) :
				Hook<5>{ a_hookedAddress, HookCodeGenerator{ a_hookedAddress } }
			{}
		};

		InitExtensionsAndNotifyHook initExtensionsAndNotifyHook{ BSScaleformManager::LoadMovie.address() + (REL::Module::IsSE() ? 0x3C2 : 0x3C0) };
		
		static DefaultTrampoline defaultTrampoline{ patchGFxMovieHook.getSize() + initExtensionsAndNotifyHook.getSize() };

		defaultTrampoline.write_branch(patchGFxMovieHook);
		GFxMovieRoot::InvokeArgs = defaultTrampoline.write_branch(initExtensionsAndNotifyHook);
	}
}
