export module cheat.csgo.interfaces:C_BaseCombatWeapon;
export import :C_BaseAnimating;
#if __has_include("C_BaseCombatWeapon_generated.ixx")
export import : C_BaseCombatWeapon_generated;
#endif

export namespace cheat::csgo
{
	class C_BaseCombatWeapon :
		public C_BaseAnimating
#if __has_include("C_BaseCombatWeapon_generated.ixx")
		, public C_BaseCombatWeapon_generated
#endif
	{

	};
}
