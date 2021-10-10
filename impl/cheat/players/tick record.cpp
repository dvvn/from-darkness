#include "tick record.h"
#include "player.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/utils/game.h"

#include "cheat/sdk/IConVar.hpp"
#include "cheat/sdk/IVEngineClient.hpp"
#include "cheat/sdk/entity/C_CSPlayer.h"

using namespace cheat;
using namespace detail;
using namespace csgo;

bones_cache_copy::bones_cache_copy(bones_cache_copy&& other) noexcept
{
	auto& l = *static_cast<base*>(this);
	auto& r = static_cast<base&>(other);

	l = r;
	r = {};
}

bones_cache_copy& bones_cache_copy::operator=(bones_cache_copy&& other) noexcept
{
	auto& l = *static_cast<base*>(this);
	auto& r = static_cast<base&>(other);

	std::swap(l, r);
	return *this;
}

bones_cache_copy::~bones_cache_copy()
{
	const auto ptr = this->data( );
	if (!ptr)
		return;
	delete[] ptr;
}

void tick_record::store_bones(C_BaseEntity* ent)
{
	runtime_assert(this->bones.data( ) == nullptr);
	const auto& bones_cache = ent->BonesCache( );
	const size_t num_bones  = bones_cache.size( );

	const auto bytes_allocated = sizeof(matrix3x4_t) * num_bones;
	const auto ptr             = new uint8_t[bytes_allocated];
	std::memcpy(ptr, bones_cache.data( ), bytes_allocated);
	static_cast<bones_cache_copy_base&>(this->bones) = {reinterpret_cast<matrix3x4_t*>(ptr), num_bones};
}

bool tick_record::is_valid(float curtime) const
{
	const auto interfaces  = csgo_interfaces::get_ptr( );
	const auto engine      = interfaces->engine.get( );
	const auto get_latency = [&](int flow)
	{
		return engine->GetNetChannelInfo( )->GetLatency(flow);
	};

	const auto correct = std::clamp(get_latency(FLOW_INCOMING) + get_latency(FLOW_OUTGOING) + utils::lerp_time( ),
									0.f, utils::unlag_limit( ));

	return std::abs(correct - (curtime - sim_time)) < utils::unlag_range( ) /*&& correct < 1.f*/;
}

tick_record::tick_record(const player& holder)
{
	#if !__has_include("cheat/sdk/generated/C_BaseEntity_h")
#pragma message(__FUNCTION__ ": skipped")
#else
	C_BaseEntity* ent = holder.ent;

	this->origin           = ent->m_vecOrigin( );
	this->abs_origin       = ent->m_vecAbsOrigin( );
	this->rotation         = ent->m_angRotation( );
	this->abs_rotation     = ent->m_angAbsRotation( );
	this->mins             = ent->m_vecMins( );
	this->maxs             = ent->m_vecMaxs( );
	this->sim_time         = holder.sim_time;
	this->coordinate_frame = reinterpret_cast<matrix3x4_t&>(ent->m_rgflCoordinateFrame( ));
#endif
}