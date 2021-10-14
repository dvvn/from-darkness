#include "tick_record.h"
#include "player.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/utils/game.h"

#include "cheat/sdk/IConVar.hpp"
#include "cheat/sdk/IVEngineClient.hpp"
#include "cheat/sdk/entity/C_CSPlayer.h"

#include <nstd/runtime_assert_fwd.h>

using namespace cheat;
using namespace detail;
using namespace csgo;

void tick_record::store_bones(C_BaseEntity* ent, std::optional<float> setup_curtime)
{
	runtime_assert(bones.empty(), "bones already cached");

	if (setup_curtime.has_value( ))
	{
		ent->SetupBones(nullptr, -1, BONE_USED_BY_ANYTHING/*BONE_USED_BY_HITBOX*/, *setup_curtime);
	}

	const auto& bones_cache = ent->BonesCache( );
	//todo: check is vector allocate more than cache.size
	bones.assign(bones_cache.begin( ), bones_cache.end( ));
}

bool tick_record::is_valid(float curtime, float correct) const
{
	/*const auto interfaces  = csgo_interfaces::get_ptr( );
	const auto engine      = interfaces->engine.get( );
	const auto get_latency = [&](int flow)
	{
		return engine->GetNetChannelInfo( )->GetLatency(flow);
	};
	const auto correct = std::clamp(get_latency(FLOW_INCOMING) + get_latency(FLOW_OUTGOING) + utils::lerp_time( ), 0.f, utils::unlag_limit( ));*/

	return std::abs(correct - (curtime - sim_time)) < utils::unlag_range( ) /*&& correct < 1.f*/;
}

tick_record::tick_record(const player& holder)
{
#if !__has_include("cheat/sdk/generated/C_BaseEntity_h")
#pragma message(__FUNCTION__ ": skipped")
#else
	C_BaseEntity* ent = holder.entptr;

	this->origin           = ent->m_vecOrigin( );
	this->abs_origin       = ent->m_vecAbsOrigin( );
	this->rotation         = ent->m_angRotation( );
	this->abs_rotation     = ent->m_angAbsRotation( );
	this->mins             = ent->m_vecMins( );
	this->maxs             = ent->m_vecMaxs( );
	this->sim_time         = holder.simtime;
	this->coordinate_frame = reinterpret_cast<matrix3x4_t&>(ent->m_rgflCoordinateFrame( ));
#endif
}
