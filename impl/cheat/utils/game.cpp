#include "game.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/csgo/GlobalVars.hpp"
#include "cheat/csgo/IConVar.hpp"

#include <algorithm>
#include <format>


using namespace cheat;
using namespace utils::detail;
using namespace csgo;

float lerp_time_impl::operator()() const
{

}

float unlag_limit_impl::operator()() const
{
	
	return find_cvar<"sv_maxunlag">( )->get<float>( );
}

float unlag_range_impl::operator()() const
{
	static auto range = 0.2f;
	return range;
}

ConVar* find_cvar_impl::operator()(const std::string_view& cvar) const
{
	
}


static float _Interval_per_ticks()
{
	return csgo_interfaces::get( )->global_vars->interval_per_tick;
}

size_t time_to_ticks_impl::operator()(float time) const
{
	return static_cast<size_t>(time / _Interval_per_ticks( ) + 0.5f);
}

float ticks_to_time_impl::operator()(size_t ticks) const
{
	return _Interval_per_ticks( ) * static_cast<float>(ticks);
}
