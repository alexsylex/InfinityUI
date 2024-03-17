#pragma once

#include "RE/G/GASString.h"
#include "RE/G/GFxPlayerStats.h"
#include "RE/G/GRefCountBase.h"
#include "RE/G/GStringHash.h"

namespace RE
{
	class GASStringManager : public GRefCountBase<GASStringManager, GFxStatMovieViews::kGFxStatMV_ActionScript_Mem>
	{
	public:
		~GASStringManager() override;  // 00

		// members
		GStringHash<GASString> unk10;  // 10
		GMemoryHeap* heap;			   // 18
		std::uint64_t unk20;		   // 20
		void* unk28;				   // 28
		std::uint64_t unk30;		   // 30
		std::uint64_t unk38;		   // 38
		std::uint64_t unk40;		   // 40
		std::uint64_t unk48;		   // 48
		std::uint64_t unk50;		   // 50
	};
	static_assert(sizeof(GASStringManager) == 0x58);
}
