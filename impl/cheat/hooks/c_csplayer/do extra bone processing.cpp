#include "do extra bone processing.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/netvars/netvars.h"
#include "cheat/players/players list.h"

using namespace cheat;
using namespace hooks;
using namespace c_csplayer;
using namespace utl;
using namespace csgo;

do_extra_bone_processing::do_extra_bone_processing( )
{
	this->Wait_for<netvars>( );
}

void do_extra_bone_processing::Load( )
{
#ifndef CHEAT_GUI_TEST

	const auto offset = _Find_signature("client.dll", "8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC").add(11).deref(1).value( ) / 4;
	this->target_func_ = method_info::make_member_virtual(bind_front(_Vtable_pointer<C_CSPlayer>,"client.dll", &csgo_interfaces::local_player), offset);

	this->hook( );
	this->enable( );
#endif
}

// ReSharper disable CppParameterNeverUsed
void do_extra_bone_processing::Callback(CStudioHdr* studio_hdr, Vector pos[], Quaternion q[], matrix3x4a_t bone_to_world[], CBoneBitList& bone_computed, CIKContext* ik_context)
{
	this->return_value_.set_original_called(true);

	//DoProceduralFootPlant also skipped here
}

// ReSharper restore CppParameterNeverUsed
