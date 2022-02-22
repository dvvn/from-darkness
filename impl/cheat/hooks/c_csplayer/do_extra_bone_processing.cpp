module;

#include "cheat/hooks/base_includes.h"
#include "cheat/players/player_includes.h"

module cheat.hooks.c_csplayer:do_extra_bone_processing;
import cheat.csgo.modules;
import cheat.players;

using namespace cheat;
using namespace csgo;
using namespace hooks::c_csplayer;

do_extra_bone_processing::do_extra_bone_processing( ) = default;

void do_extra_bone_processing::construct( ) noexcept
{
	this->deps( ).add<csgo_interfaces>( );
}

bool do_extra_bone_processing::load( ) noexcept
{
	const csgo_interface vtable = csgo_modules::client->find_vtable<C_CSPlayer>( );
	const auto index = csgo_modules::client->find_signature("8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC").add(11).deref(1) / 4;
	this->set_target_method(vtable.vfunc(index.value));
	return hook_base::load( );
}

void do_extra_bone_processing::callback(CStudioHdr * studio_hdr, Vector pos[],
										Quaternion q[], matrix3x4a_t bone_to_world[],
										CBoneBitList & bone_computed, CIKContext * ik_context)
{
	this->store_return_value( );
}
