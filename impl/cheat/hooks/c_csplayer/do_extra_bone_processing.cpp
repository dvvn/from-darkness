#include "do_extra_bone_processing.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::c_csplayer;

void do_extra_bone_processing_impl::callback(CStudioHdr* studio_hdr, Vector pos[],
                                             Quaternion q[], matrix3x4a_t bone_to_world[],
                                             CBoneBitList& bone_computed, CIKContext* ik_context)
{
    this->store_return_value( );
}
