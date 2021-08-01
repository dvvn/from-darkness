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
}

bool do_extra_bone_processing::Do_load( )
{
#ifdef CHEAT_GUI_TEST

	return 0;
#else

	using namespace address_pipe;

	this->target_func_ = method_info::make_member_virtual
			(
			 bind_front(_Vtable_pointer<C_CSPlayer>, "client.dll"),
			 bind_front(_Find_signature, "client.dll", "8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC") | add(11) | deref(1) | divide(4) | value
			);

	this->hook( );
	this->enable( );

	return 1;
#endif
}

void do_extra_bone_processing::Callback([[maybe_unused]] CStudioHdr* studio_hdr, [[maybe_unused]] Vector pos[],
										[[maybe_unused]] Quaternion q[], [[maybe_unused]] matrix3x4a_t bone_to_world[],
										[[maybe_unused]] CBoneBitList& bone_computed, [[maybe_unused]] CIKContext* ik_context)
{
	this->return_value_.set_original_called();

	//DoProceduralFootPlant also skipped here
}
