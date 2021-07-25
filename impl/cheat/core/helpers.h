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
			using sv = utl::string_view;
			using mb = utl::memory_block;

			utl::address operator()(const mb& from, const sv& sig) const;
			utl::address operator()(const sv& dll_name, const sv& sig) const;
		};
	}

	inline constexpr auto _Find_signature = detail::_Find_signature_impl( );

	namespace detail
	{
		template <typename T>
		struct type_name
		{
			using sv = utl::string_view;

		private:
			static constexpr sv Drop_namespace_(const sv& str)
			{
				size_t end;
				if (const auto internal_template_start = str.find('<'); internal_template_start == str.npos)
				{
					end = str.rfind(':');
				}
				else
				{
					const auto str2 = str.substr(0, internal_template_start - 1);
					end = str2.rfind(':');
				}

				if (end == str.npos)
					return str;

				return str.substr(end + 1);
			}

		public:
			constexpr sv operator()(bool drop_namespace = false) const noexcept
			{
				constexpr auto full_name = sv(__FUNCSIG__);
				constexpr auto left_marker = sv("type_name<");
				constexpr auto right_marker = sv(">::operator");

				constexpr auto left_marker_index = full_name.find(left_marker);
				//static_assert(left_marker_index != std::string_view::npos);
				constexpr auto start_index = left_marker_index + left_marker.size( );
				constexpr auto end_index = full_name.find(right_marker, left_marker_index);
				//static_assert(end_index != std::string_view::npos);
				constexpr auto length = end_index - start_index;

				//class cheat::A::B:: name
				constexpr auto obj_name = full_name.substr(start_index, length);
				constexpr auto left_marker2 = sv("cheat::");

				constexpr auto ret_val = [&]
				{
					if constexpr (constexpr auto left_marker_index2 = obj_name.find(left_marker2); left_marker_index2 != sv::npos)
					{
						return obj_name.substr(left_marker_index2 + left_marker2.size( ));
					}
					else
					{
						constexpr auto space_index = obj_name.find(' ');
						return space_index == sv::npos ? obj_name : obj_name.substr(space_index + 1);
					}
				}( );
				constexpr auto ret_val_namespaces_dropped = Drop_namespace_(ret_val);

				return drop_namespace ? ret_val_namespaces_dropped : ret_val;
			}
		};
	}

	template <typename T>
	constexpr utl::string_view _Type_name(bool drop_namespace = false)
	{
		return utl::invoke(detail::type_name<T>( ), drop_namespace);
	}

	namespace detail
	{
		void* _Vtable_pointer_get(const utl::string_view& from, const utl::string_view& table_name, const utl::function<void*(csgo_interfaces*)>& preferred);

		template <typename T>
		struct _Vtable_pointer_impl
		{
			template <typename Pr=std::false_type>
			T* operator()(const utl::string_view& from /*= { }*/,Pr preferred_interface = { },  bool drop_namespaces = true) const
			{
				auto table_name = _Type_name<T>(drop_namespaces);

				void* ptr;

				if constexpr (!std::invocable<decltype(preferred_interface), csgo_interfaces*>)
				{
					ptr = _Vtable_pointer_get(from, table_name, { });
				}
				else
				{
					ptr = _Vtable_pointer_get(from, table_name, [=](csgo_interfaces* ifcs)-> void*
					{
						auto obj = utl::invoke(preferred_interface, ifcs);
						return obj == nullptr ? nullptr : obj.get( );
					});
				}

				return static_cast<T*>(ptr);
			}
		};
	}

	template<typename T>
	inline constexpr auto _Vtable_pointer = detail::_Vtable_pointer_impl<T>( );
}
