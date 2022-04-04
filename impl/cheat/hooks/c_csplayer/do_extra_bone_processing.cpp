module;

#include <cheat/hooks/console_log.h>
#include "cheat/players/player_includes.h"

module cheat.hooks.c_csplayer.do_extra_bone_processing;
import cheat.csgo.modules;
import cheat.players;
import cheat.console.object_message;

using namespace cheat;
using namespace csgo;
using namespace hooks::c_csplayer;

do_extra_bone_processing::do_extra_bone_processing( )
{
	const nstd::mem::basic_address vtable_holder = csgo_modules::client.find_vtable<C_CSPlayer>( );
	const auto index = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC">( ).plus(11).deref<1>( ).divide(4);
	this->set_target_method(vtable_holder.deref<1>( )[index.value]);
}

CHEAT_HOOKS_CONSOLE_LOG(do_extra_bone_processing);

void do_extra_bone_processing::callback(CStudioHdr* studio_hdr, Vector pos[],
										Quaternion q[], matrix3x4a_t bone_to_world[],
										CBoneBitList& bone_computed, CIKContext* ik_context)
{
	this->store_return_value( );
}
