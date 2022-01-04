export module cheat.csgo.interfaces:C_BaseCombatCharacter;
export import :C_BaseAnimating;
#if __has_include("C_BaseCombatCharacter_generated.ixx")
export import : C_BaseCombatCharacter_generated;
#endif

export namespace cheat::csgo
{
	class C_BaseCombatWeapon;

	class C_BaseCombatCharacter :
		public C_BaseAnimating
#if __has_include("C_BaseCombatCharacter_generated.ixx")
		, public C_BaseCombatCharacter_generated
#endif
	{
	public:
		C_BaseCombatWeapon* GetActiveWeapon( );
	};
}
