module;

#include <memory>

module cheat.netvars.storage;
import cheat.csgo.interfaces.C_BaseAnimating;
import cheat.csgo.modules;
import nstd.mem.address;

using namespace cheat;
using namespace netvars;
using namespace csgo;

void storage::store_handmade_netvars( )
{
	const auto baseent = this->find<C_BaseEntity>( );
	baseent->add<VarMapping_t>(0x24, "m_InterpVarMap");
	baseent->add<matrix3x4_t>([]
	{
		return csgo_modules::client.find_signature<"8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8">( ).plus(9).deref<1>( ).minus(8u);
	}, "m_BonesCache", type_utlvector);

	const auto baseanim = this->find<C_BaseAnimating>( );
	//m_vecRagdollVelocity - 128
	baseanim->add<CAnimationLayer>([]
	{
		return csgo_modules::client.find_signature<"8B 87 ? ? ? ? 83 79 04 00 8B">( ).plus(2).deref<1>( );
	}, "m_AnimOverlays", type_utlvector);
	//m_hLightingOrigin - 32
	baseanim->add<float>([]
	{
		return csgo_modules::client.find_signature<"C7 87 ? ? ? ? ? ? ? ? 89 87 ? ? ? ? 8B 8F">( ).plus(2).deref<1>( );
	}, "m_flLastBoneSetupTime");
	//m_nForceBone + 4
	baseanim->add<int>([]
	{
		return csgo_modules::client.find_signature<"89 87 ? ? ? ? 8B 8F ? ? ? ? 85 C9 74 10">( ).plus(2).deref<1>( );
	}, "m_iMostRecentModelBoneCounter");

	//_Load_class<C_BasePlayer>( );
}
