#pragma once
#include "nstd/chars cache.h"

#include <string_view>

namespace cheat::csgo
{
	class ConVar;
}

namespace cheat::utils
{
	namespace detail
	{
		struct lerp_time_impl
		{
			float operator()( ) const;
		};
		struct unlag_limit_impl
		{
			float operator()( ) const;
		};
		struct unlag_range_impl
		{
			float operator()( ) const;
		};
	}

	_INLINE_VAR constexpr auto lerp_time   = detail::lerp_time_impl( );
	_INLINE_VAR constexpr auto unlag_limit = detail::unlag_limit_impl( );
	_INLINE_VAR constexpr auto unlag_range = detail::unlag_range_impl( );

	namespace detail
	{
		struct find_cvar_impl
		{
			csgo::ConVar* operator()(const std::string_view& cvar) const;
		};
	}

	template <nstd::chars_cache Name>
	csgo::ConVar* find_cvar( )
	{
		static auto cvar = std::invoke(detail::find_cvar_impl( ), Name.view( ));
		return cvar;
	}

	namespace detail
	{
		struct time_to_ticks_impl
		{
			size_t operator()(float time) const;
		};

		struct ticks_to_time_impl
		{
			float operator()(size_t ticks) const;
		};
	}

	_INLINE_VAR constexpr auto time_to_ticks = detail::time_to_ticks_impl( );
	_INLINE_VAR constexpr auto ticks_to_time = detail::ticks_to_time_impl( );
}
