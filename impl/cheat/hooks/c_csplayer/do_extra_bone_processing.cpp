#include "do_extra_bone_processing.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::c_csplayer;

void do_extra_bone_processing::callback(CStudioHdr* studio_hdr, Vector pos[],
										Quaternion q[], matrix3x4a_t bone_to_world[],
										CBoneBitList& bone_computed, CIKContext* ik_context)
{
#if !CHEAT_MODE_INGAME
	CHEAT_CALL_BLOCKER
#else
	this->return_value_.set_original_called( );

	//DoProceduralFootPlant also skipped here
#endif
}
