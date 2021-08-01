#pragma once
#include "cheat/sdk/IConVar.hpp"
#include "cheat/sdk/entity/C_CSPlayer.h"

namespace cheat
{
	class csgo_interfaces;
	//--

	float _Lerp_time( );

	float _Unlag_limit( );
	float _Unlag_range( );

	csgo::ConVar* _Find_cvar(const utl::string_view& cvar);

	size_t _Time_to_ticks(float time);
	float _Ticks_to_time(size_t ticks);

	namespace detail
	{
		struct _Find_signature_impl
		{
			utl::address operator()(const utl::memory_block& from, const utl::string_view& sig) const;
			utl::address operator()(const utl::string_view& dll_name, const utl::string_view& sig) const;
		};
	}

	inline constexpr auto _Find_signature = detail::_Find_signature_impl( );

	template <typename T>
	constexpr utl::string_view _Type_name(bool drop_namespace = false)
	{
		constexpr auto full_name = utl::string_view(__FUNCSIG__);
		constexpr auto left_marker = utl::string_view("_Type_name<");
		constexpr auto right_marker = utl::string_view(">(");

		constexpr auto left_marker_index = full_name.find(left_marker);
		//static_assert(left_marker_index != std::string_view::npos);
		constexpr auto start_index = left_marker_index + left_marker.size( );
		constexpr auto end_index = full_name.find(right_marker, left_marker_index);
		//static_assert(end_index != std::string_view::npos);
		constexpr auto length = end_index - start_index;

		//class cheat::A::B:: name
		constexpr auto obj_name = full_name.substr(start_index, length);
		constexpr auto left_marker2 = utl::string_view("cheat::");

		constexpr auto ret_val = [&]
		{
			if constexpr (constexpr auto left_marker_index2 = obj_name.find(left_marker2); left_marker_index2 != utl::string_view::npos)
			{
				return obj_name.substr(left_marker_index2 + left_marker2.size( ));
			}
			else
			{
				constexpr auto space_index = obj_name.find(' ');
				return space_index == utl::string_view::npos ? obj_name : obj_name.substr(space_index + 1);
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
				end = ret_val2.rfind(':');
			}

			if (end == ret_val.npos)
				return ret_val;

			return ret_val.substr(end + 1);
		}( );

		return drop_namespace ? ret_val_namespaces_dropped : ret_val;
	}

	namespace detail
	{
		void* _Vtable_pointer_impl(const utl::string_view& from, const utl::string_view& table_name);

		template <typename T>
		struct _Vtable_pointer_fn
		{
			T* operator()(const utl::string_view& from, bool drop_namespaces = true) const
			{
				const auto table_name = _Type_name<T>(drop_namespaces);
				void* ptr = _Vtable_pointer_impl(from, table_name);
				return static_cast<T*>(ptr);
			}
		};
	}
	template <typename T>
	inline constexpr auto _Vtable_pointer = detail::_Vtable_pointer_fn<T>( );
}
