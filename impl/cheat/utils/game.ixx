module;

#include <nstd/chars cache.h>

export module cheat.utils.game;

export namespace cheat::csgo
{
	class ConVar;
}

export namespace cheat::utils
{
	cheat::csgo::ConVar* find_cvar(const std::string_view& cvar);

	template<nstd::chars_cache Cvar>
	auto find_cvar( )
	{
		static auto cvar = find_cvar(Cvar.view( ));
		return cvar;
	}

	float lerp_time( );
	float unlag_limit( );
	float unlag_range( );

	size_t time_to_ticks(float time);
	float ticks_to_time(size_t ticks);
}