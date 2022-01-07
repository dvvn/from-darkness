module;

#include "cheat/console/includes.h"

#include <nstd/mem/address_includes.h>
#include <nstd/chars cache.h>
#include <nstd/format.h>

#include <string>
#include <algorithm>

module cheat.utils.game;
import cheat.csgo.interfaces;
import cheat.console;
import nstd.mem;

using namespace cheat;
using namespace csgo;
using nstd::mem::address;

ConVar* utils::find_cvar(const std::string_view& cvar)
{
	address cvars = csgo_interfaces::get( )->cvars.get( );
	ConVar* root_cvar = cvars.add(0x30).deref(1).ptr( );
	ConVar* target_cvar = 0;

	for (auto cv = root_cvar; cv != nullptr; cv = cv->m_pNext)
	{
		if (std::strncmp(cv->m_pszName, cvar.data( ), cvar.size( )) != 0)
			continue;

		target_cvar = cv;
		break;
	}

	console::get( )->log("Cvar \"{}\"{}found", cvar, target_cvar ? " not " : " ");
	return target_cvar;
}

float utils::lerp_time( )
{
	//using utils::find_cvar;

#if 0
	const auto cl_interp = m_cvar( )->FindVar(crypt_str("cl_interp"));
	const auto cl_interp_ratio = m_cvar( )->FindVar(crypt_str("cl_interp_ratio"));
	const auto sv_client_min_interp_ratio = m_cvar( )->FindVar(crypt_str("sv_client_min_interp_ratio"));
	const auto sv_client_max_interp_ratio = m_cvar( )->FindVar(crypt_str("sv_client_max_interp_ratio"));
	const auto cl_updaterate = m_cvar( )->FindVar(crypt_str("cl_updaterate"));
	const auto sv_minupdaterate = m_cvar( )->FindVar(crypt_str("sv_minupdaterate"));
	const auto sv_maxupdaterate = m_cvar( )->FindVar(crypt_str("sv_maxupdaterate"));

	auto updaterate = math::clamp(cl_updaterate, sv_minupdaterate, sv_maxupdaterate);
	auto lerp_ratio = math::clamp(cl_interp_ratio, sv_client_min_interp_ratio, sv_client_max_interp_ratio);

	return math::clamp(lerp_ratio / updaterate, cl_interp, 1.0f);
#endif

#if 0

	const auto cl_interp = find_cvar<"cl_interp">( );
	const auto cl_updaterate = find_cvar<"cl_updaterate">( );
	const auto cl_interp_ratio = find_cvar<"cl_interp_ratio">( );

	const auto a2 = cl_updaterate;
	const auto a1 = cl_interp;
	const auto v2 = cl_interp_ratio / a2;

	return fmaxf(a1, v2);

#endif

	const auto update_rate = std::clamp(find_cvar<"cl_updaterate">( )->get<float>( ), find_cvar<"sv_minupdaterate">( )->get<float>( ), find_cvar<"sv_maxupdaterate">( )->get<float>( ));
	auto lerp_ratio = find_cvar<"cl_interp_ratio">( )->get<float>( );

	if (lerp_ratio == 0.0f)
		lerp_ratio = 1.0f;

	const auto lerp_amount = find_cvar<"cl_interp">( )->get<float>( );

	const auto min_ratio = find_cvar<"sv_client_min_interp_ratio">( )->get<float>( );
	const auto max_ratio = find_cvar<"sv_client_max_interp_ratio">( )->get<float>( );

	if (min_ratio != -1.0f)
		lerp_ratio = std::clamp(lerp_ratio, min_ratio, max_ratio);

	const auto ret = std::max(lerp_amount, lerp_ratio / update_rate);
	return ret;
}

float utils::unlag_limit( )
{
	return find_cvar<"sv_maxunlag">( )->get<float>( );
}

float utils::unlag_range( )
{
	static auto range = 0.2f;
	return range;
}

size_t utils::time_to_ticks(float time)
{
	const auto interval = csgo_interfaces::get( )->global_vars->interval_per_tick;
	return static_cast<size_t>(time / interval + 0.5f);
}

float utils::ticks_to_time(size_t ticks)
{
	const auto interval = csgo_interfaces::get( )->global_vars->interval_per_tick;
	return interval * static_cast<float>(ticks);
}
