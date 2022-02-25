module;

#include <cstdint>

export module cheat.csgo.interfaces.C_BaseAnimating;
export import cheat.csgo.interfaces.C_BaseEntity;
export import cheat.csgo.interfaces.AnimationLayer;

export namespace cheat::csgo
{
	class QuaternionAligned;
	class CStudioHdr;
	class CBoneBitList;
	class CIKContext;

	class C_BaseAnimating : public C_BaseEntity
	{
	public:

#if __has_include("C_BaseAnimating_generated_h")
#include "C_BaseAnimating_generated_h"
#endif

		void UpdateClientSideAnimation( );
		void InvalidateBoneCache( );
	};
}
