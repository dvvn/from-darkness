#pragma once

namespace nstd
{
	//use it for as template parameter

	template <std::size_t N,typename Chr>
	struct chars_cache
	{
		std::array<Chr, N> cache;

		using value_type = Chr;

		template <std::size_t...Is>
		constexpr chars_cache(const Chr (&arr)[N], std::index_sequence<Is...>) : cache{arr[Is]...}
		{
		}

		constexpr chars_cache(const Chr (&arr)[N]) : chars_cache(arr, std::make_index_sequence<N>( ))
		{
		}

		constexpr bool empty( ) const
		{
			return cache.front( ) == static_cast<Chr>('\0');
		}

		static constexpr size_t size = N;

		constexpr std::basic_string_view<Chr> view( ) const
		{
			return {cache._Unchecked_begin( ), N - 1};
		}
	};
}
