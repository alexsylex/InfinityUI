#pragma once

#include "RE/G/GASObject.h"

namespace RE
{
	class GASFunctionObject : public GASObject
	{
	public:
		~GASFunctionObject() override;	// 02

		// override (GASObject)
		ObjectType GetObjectType2() const override;	 // 02 - { return ObjectType::kFunction; }
		GASFunctionRef ToFunction() override;		 // 0C

		// add
		virtual void Unk_09(void);	// 09 - pure (GetExecutionContext?)
		virtual void Unk_0A(void);	// 0A - pure (Execute?)
		virtual void Unk_0B(void);	// 0B - pure
		virtual void Unk_0C(void);	// 0C - pure
		virtual void Unk_0D(void);	// 0D
		virtual void Unk_0E(void);	// 0E
		virtual void Unk_0F(void);	// 0F
		virtual void Unk_10(void);	// 10 - pure
	};
	static_assert(sizeof(GASFunctionObject) == 0x68);
}
