export module cheat.csgo.interfaces:C_CSPlayer;
export import :C_BasePlayer;
#if __has_include("C_CSPlayer_generated.ixx")
export import : C_CSPlayer_generated;
#endif

export namespace cheat::csgo
{
	////econ
	//class CAttributeManager;
	//class CAttributeList;
	//class CAttributeContainer;

	class C_CSPlayer :
		public C_BasePlayer
#if __has_include("C_CSPlayer_generated.ixx")
		, public C_CSPlayer_generated
#endif
	{
	public:
		C_BaseAnimating* GetRagdoll( );
	};
}
