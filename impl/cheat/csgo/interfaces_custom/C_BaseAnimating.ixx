module;

#include <cstdint>

export module cheat.csgo.interfaces:C_BaseAnimating;
export import :C_BaseEntity;
#if __has_include("C_BaseAnimating_generated.ixx")
export import : C_BaseAnimating_generated;
#endif

export namespace cheat::csgo
{
	class QuaternionAligned;
	class CStudioHdr;
	class CBoneBitList;
	class CIKContext;

	

	class C_BaseAnimating :
		public C_BaseEntity
#if __has_include("C_BaseAnimating_generated.ixx")
		, public C_BaseAnimating_generated
#endif
	{
	public:
		void UpdateClientSideAnimation( );
		void InvalidateBoneCache( );
	};
}
