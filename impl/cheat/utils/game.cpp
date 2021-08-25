#include "game.h"

#include "cheat/sdk/GlobalVars.hpp"
#include "cheat/sdk/IConVar.hpp"

using namespace cheat;
using namespace utils::detail;
using namespace csgo;

float lerp_time_impl::operator()( ) const
{
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

	const auto cl_interp = find_cvar<"cl_interp">();
	const auto cl_updaterate = find_cvar<"cl_updaterate">();
	const auto cl_interp_ratio = find_cvar<"cl_interp_ratio">();

	const auto a2 = cl_updaterate;
	const auto a1 = cl_interp;
	const auto v2 = cl_interp_ratio / a2;

	return fmaxf(a1, v2);

#endif

	const auto update_rate = std::clamp(find_cvar<"cl_updaterate">( )->get<float>( ),
										find_cvar<"sv_minupdaterate">( )->get<float>( ),
										find_cvar<"sv_maxupdaterate">( )->get<float>( ));
	float lerp_ratio = find_cvar<"cl_interp_ratio">( )->get<float>( );

	if (lerp_ratio == 0.0f)
		lerp_ratio = 1.0f;

	const float lerp_amount = find_cvar<"cl_interp">( )->get<float>( );

	const float min_ratio = find_cvar<"sv_client_min_interp_ratio">( )->get<float>( );
	const float max_ratio = find_cvar<"sv_client_max_interp_ratio">( )->get<float>( );

	if (static_cast<float>(min_ratio) != -1.0f)
		lerp_ratio = std::clamp<float>(lerp_ratio, min_ratio, max_ratio);

	const auto ret = std::max(lerp_amount, lerp_ratio / update_rate);
	return ret;
}

float unlag_limit_impl::operator()( ) const
{
	return find_cvar<"sv_maxunlag">( )->get<float>( );
}

float unlag_range_impl::operator()( ) const
{
	static auto range = 0.2f;
	return range;
}

ConVar* find_cvar_impl::operator()(const std::string_view& cvar) const
{
	constexpr auto get_root_cvar = []
	{
		const auto cvars = csgo_interfaces::get_ptr( )->cvars.get( );
		return nstd::address(cvars).add(0x30).deref(1).ptr<ConVar>( );
	};
	const auto get_cvar_from_game = [&]( )-> ConVar*
	{
		for (auto cv = get_root_cvar( ); cv != nullptr; cv = cv->m_pNext)
		{
			if (cv->m_pszName == cvar)
				return cv;
		}

		return nullptr;
	};

	const auto cv = get_cvar_from_game( );
	CHEAT_CONSOLE_LOG("Cvar {} {}", cvar, cv == nullptr ? "not found" : "found");
	return cv;
}

static float _Interval_per_ticks( )
{
	return csgo_interfaces::get_ptr( )->global_vars->interval_per_tick;
}

size_t time_to_ticks_impl::operator()(float time) const
{
	return static_cast<size_t>(time / _Interval_per_ticks( ) + 0.5f);
}

float ticks_to_time_impl::operator()(size_t ticks) const
{
	return _Interval_per_ticks( ) * static_cast<float>(ticks);
}
