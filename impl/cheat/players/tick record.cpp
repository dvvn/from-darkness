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

bool tick_record::is_valid(float curtime) const
{
	const auto get_latency = [engine = csgo_interfaces::get( ).engine.get( )](int flow)
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
	auto ent = holder.ent;
	auto tick = this->get( );

	(void)tick;

	tick->origin = invoke(&C_BaseEntity::m_vecOrigin, ent);
	tick->abs_origin = ent->m_vecAbsOrigin( );
	tick->rotation = ent->m_angRotation( );
	tick->abs_rotation = ent->m_angAbsRotation( );
	tick->mins = ent->m_vecMins( );
	tick->maxs = ent->m_vecMaxs( );
	tick->sim_time = holder.sim_time;
	tick->coordinate_frame = reinterpret_cast<matrix3x4_t&>(ent->m_rgflCoordinateFrame( ));
#endif
}
