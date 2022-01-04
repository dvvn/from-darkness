export module cheat.csgo.interfaces:C_WeaponCSBase;
export import :C_BaseCombatWeapon;
#if __has_include("C_WeaponCSBase_generated.ixx")
export import : C_WeaponCSBase_generated;
#endif

namespace cheat::csgo
{
	class C_WeaponCSBase :
		public C_BaseCombatWeapon
#if __has_include("C_WeaponCSBase_generated.ixx")
		, public C_WeaponCSBase_generated
#endif
	{

	};
}
