//#include "tick_record.h"
//#include "player.h"
//
//#include "cheat/core/csgo_interfaces.h"
//#include "cheat/utils/game.h"
//
//#include "cheat/csgo/IConVar.hpp"
//#include "cheat/csgo/IVEngineClient.hpp"
//#include "cheat/csgo/entity/C_CSPlayer.h"
//
//#include <nstd/runtime_assert_fwd.h>
//
//#include <optional>

module;

#include "player_includes.h"

#include <nstd/runtime_assert.h>

module cheat.players:tick_record;
import :player;
import cheat.utils.game;

using namespace cheat;
using namespace csgo;

void tick_record::store_bones(C_BaseEntity* ent, const std::optional<float>& setup_curtime)
{
#if 0
	runtime_assert(bones.empty( ), "bones already cached");

	if (setup_curtime.has_value( ))
	{
		ent->SetupBones(nullptr, -1, BONE_USED_BY_ANYTHING/*BONE_USED_BY_HITBOX*/, *setup_curtime);
	}

	const auto& bones_cache = ent->m_BonesCache( );
	bones.assign(bones_cache.begin( ), bones_cache.end( ));
#endif
}

void tick_record::store_animations(C_BaseAnimating* ent)
{

	/*runtime_assert(layers.empty( ), "layers already cached");
	const auto& layers0 = ent->m_AnimOverlays( );
	layers.assign(layers0.begin( ), layers0.end( ));

	runtime_assert(poses.empty( ), "poses already cached");
	const auto& poses0 = ent->m_flPoseParameter( );
	poses.assign(poses0.begin( ), poses0.end( ));*/

}

bool tick_record::is_valid(float curtime, float correct) const
{
	return std::abs(correct - (curtime - sim_time)) < 0.2f;
}

tick_record::tick_record(const player& holder)
{
	/*C_BaseEntity* ent = holder.entptr;

	this->origin = ent->m_vecOrigin( );
	this->abs_origin = ent->m_vecAbsOrigin( );
	this->rotation = ent->m_angRotation( );
	this->abs_rotation = ent->m_angAbsRotation( );
	this->mins = ent->m_vecMins( );
	this->maxs = ent->m_vecMaxs( );
	this->sim_time = *holder.simtime;
	this->coordinate_frame = reinterpret_cast<matrix3x4_t&>(ent->m_rgflCoordinateFrame( ));*/
}
