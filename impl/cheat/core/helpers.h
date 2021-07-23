#pragma once

namespace cheat
{
	namespace csgo
	{
		class ConVar;
	}

	float _Lerp_time( );

	float _Unlag_limit( );
	float _Unlag_range( );

	csgo::ConVar* _Find_cvar(const utl::string_view& cvar);

	size_t _Time_to_ticks(float time);
	float _Ticks_to_time(size_t ticks);
}
