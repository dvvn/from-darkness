#include "tick record.h"
#include "player.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/core/helpers.h"

#include "cheat/sdk/IConVar.hpp"
#include "cheat/sdk/IVEngineClient.hpp"
#include "cheat/sdk/entity/C_CSPlayer.h"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;

stored_player_bones::stored_player_bones([[maybe_unused]] C_BaseEntity* ent)
{
	const auto&  bones     = ent->BonesCache( );
	const size_t num_bones = bones.size( );

	cache__ = std::make_unique<matrix3x4_t[]>(num_bones);
	std::memcpy(cache__.get( ), bones.data( ), num_bones * sizeof(matrix3x4_t));

	*static_cast<span*>(this) = {cache__.get( ), num_bones};
}

bool tick_record::is_valid(float curtime) const
{
	const auto interfaces  = csgo_interfaces::get_ptr( );
	const auto engine      = interfaces->engine.get( );
	const auto get_latency = [&](int flow)
	{
		return engine->GetNetChannelInfo( )->GetLatency(flow);
	};

	const auto correct = std::clamp(get_latency(FLOW_INCOMING) +
									get_latency(FLOW_OUTGOING)
									+ _Lerp_time( ), 0.f, _Unlag_limit( ));

	return std::abs(correct - (curtime - sim_time)) < _Unlag_range( ) /*&& correct < 1.f*/;
}

void tick_record_shared_impl::init(const player& holder)
{
	shared_holder::init( );

#ifndef CHEAT_NETVARS_UPDATING
	C_BaseEntity* ent  = holder.ent;
	const auto    tick = this->get( );

	tick->origin           = ent->m_vecOrigin( );
	tick->abs_origin       = ent->m_vecAbsOrigin( );
	tick->rotation         = ent->m_angRotation( );
	tick->abs_rotation     = ent->m_angAbsRotation( );
	tick->mins             = ent->m_vecMins( );
	tick->maxs             = ent->m_vecMaxs( );
	tick->sim_time         = holder.sim_time;
	tick->coordinate_frame = reinterpret_cast<matrix3x4_t&>(ent->m_rgflCoordinateFrame( ));
#endif
}

void tick_record_shared_impl::store_bones(C_BaseEntity* ent)
{
	auto& bones = this->get( )->bones;
	runtime_assert(bones.empty( ));
	bones = {ent};

	(void)this;
}
