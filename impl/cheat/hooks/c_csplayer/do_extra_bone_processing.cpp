module;

#include <cheat/hooks/instance.h>

module cheat.hooks.c_csplayer.do_extra_bone_processing;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_CSPlayer;
import cheat.hooks.base;
import nstd.one_instance;

using namespace cheat;
using namespace csgo;
using namespace hooks;

using do_extra_bone_processing_base = hooks::base<void(C_BaseAnimating::*)(CStudioHdr*, Vector*, Quaternion*, matrix3x4a_t*, CBoneBitList&, CIKContext*)>;
struct do_extra_bone_processing_impl :do_extra_bone_processing_base
{
	do_extra_bone_processing_impl( )
	{
		const nstd::mem::basic_address vtable_holder = csgo_modules::client.find_vtable<C_CSPlayer>( );
		const auto index = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC">( ).plus(11).deref<1>( ).divide(4);
		this->set_target_method(vtable_holder.deref<1>( )[index.value]);
	}

	void callback(CStudioHdr* studio_hdr, Vector pos[], Quaternion q[], matrix3x4a_t bone_to_world[], CBoneBitList& bone_computed, CIKContext* ik_context)
	{
		this->store_return_value( );
	}
};

CHEAT_HOOK_INSTANCE(c_csplayer, do_extra_bone_processing);


