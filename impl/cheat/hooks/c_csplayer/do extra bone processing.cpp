#include "do extra bone processing.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/netvars/netvars.h"
#include "cheat/players/players list.h"

using namespace cheat;
using namespace hooks;
using namespace c_csplayer;
using namespace utl;
using namespace csgo;

utl::address do_extra_bone_processing::get_target_method_impl( ) const
{
	const auto vtable = vtable_pointer<C_CSPlayer>("client.dll");
	const auto index  = find_signature("client.dll", "8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC").add(11).deref(1).divide(4).value( );

	return _Pointer_to_virtual_class_table(vtable)[index];
}

void do_extra_bone_processing::callback([[maybe_unused]] CStudioHdr*   studio_hdr, [[maybe_unused]] Vector         pos[],
										[[maybe_unused]] Quaternion    q[], [[maybe_unused]] matrix3x4a_t          bone_to_world[],
										[[maybe_unused]] CBoneBitList& bone_computed, [[maybe_unused]] CIKContext* ik_context)
{
	this->return_value_.set_original_called( );

	//DoProceduralFootPlant also skipped here
}
