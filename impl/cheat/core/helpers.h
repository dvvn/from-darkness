#pragma once
#include "cheat/sdk/IConVar.hpp"
#include "cheat/sdk/entity/C_CSPlayer.h"

namespace cheat
{
	class csgo_interfaces;
	//--

	float lerp_time( );

	float unlag_limit( );
	float unlag_range( );

	namespace detail
	{
		csgo::ConVar* find_cvar_impl(const std::string_view& cvar);

		template <std::size_t N>
		struct chars_cache
		{
			std::array<char, N> cache;

			template <std::size_t...Is>
			constexpr chars_cache(const char (&arr)[N], std::index_sequence<Is...>) : cache{arr[Is]...}
			{
			}

			constexpr chars_cache(const char (&arr)[N]) : chars_cache(arr, std::make_index_sequence<N>( ))
			{
			}

			constexpr std::string_view view( ) const
			{
				return {cache._Unchecked_begin( ), N - 1};
			}
		};
	}

	template <detail::chars_cache Name>
	csgo::ConVar* find_cvar( )
	{
		static auto cvar = detail::find_cvar_impl(Name.view( ));
		return cvar;
	}

	size_t time_to_ticks(float time);
	float  ticks_to_time(size_t ticks);

	namespace detail
	{
		struct find_signature_impl
		{
			utl::address operator()(const utl::memory_block& from, const std::string_view& sig) const;
			utl::address operator()(const std::string_view& dll_name, const std::string_view& sig) const;
		};
	}

	inline constexpr auto find_signature = detail::find_signature_impl( );

	template <typename T>
	constexpr std::string_view type_name(bool drop_namespace = false)
	{
		constexpr auto full_name    = std::string_view(__FUNCSIG__);
		constexpr auto left_marker  = std::string_view("type_name<");
		constexpr auto right_marker = std::string_view(">(");

		constexpr auto left_marker_index = full_name.find(left_marker);
		//static_assert(left_marker_index != std::string_view::npos);
		constexpr auto start_index = left_marker_index + left_marker.size( );
		constexpr auto end_index   = full_name.find(right_marker, left_marker_index);
		//static_assert(end_index != std::string_view::npos);
		constexpr auto length = end_index - start_index;

		//class cheat::A::B:: name
		constexpr auto obj_name     = full_name.substr(start_index, length);
		constexpr auto left_marker2 = std::string_view("cheat::");

		constexpr auto ret_val = [&]
		{
			if constexpr (constexpr auto left_marker_index2 = obj_name.find(left_marker2); left_marker_index2 != std::string_view::npos)
			{
				return obj_name.substr(left_marker_index2 + left_marker2.size( ));
			}
			else
			{
				constexpr auto space_index = obj_name.find(' ');
				return space_index == std::string_view::npos ? obj_name : obj_name.substr(space_index + 1);
			}
		}( );
		constexpr auto ret_val_namespaces_dropped = [&]
		{
			size_t end;
			if (const auto internal_template_start = ret_val.find('<'); internal_template_start == ret_val.npos)
			{
				end = ret_val.rfind(':');
			}
			else
			{
				const auto ret_val2 = ret_val.substr(0, internal_template_start - 1);
				end                 = ret_val2.rfind(':');
			}

			if (end == ret_val.npos)
				return ret_val;

			return ret_val.substr(end + 1);
		}( );

		return drop_namespace ? ret_val_namespaces_dropped : ret_val;
	}

	namespace detail
	{
		void* vtable_pointer_impl(const std::string_view& from, const std::string_view& table_name);

		template <typename T>
		struct vtable_pointer_fn
		{
			auto operator()(const std::string_view& from, bool drop_namespaces = true) const
			{
				const auto table_name = type_name<T>(drop_namespaces);
				void*      ptr        = vtable_pointer_impl(from, table_name);
				return static_cast<T*>(ptr);
			}
		};
	}

	template <typename T>
	inline constexpr auto vtable_pointer = detail::vtable_pointer_fn<T>( );
}
