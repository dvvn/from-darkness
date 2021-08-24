#pragma once
#include "cheat/sdk/IConVar.hpp"
#include "cheat/sdk/entity/C_CSPlayer.h"

namespace cheat
{
	class csgo_interfaces;
}

namespace cheat::utils
{
	namespace detail
	{
		struct find_signature_impl
		{
			nstd::address operator()(const nstd::memory_block& from, const std::string_view& sig) const;
			nstd::address operator()(const std::string_view& dll_name, const std::string_view& sig) const;
		};
	}

	_INLINE_VAR constexpr auto find_signature = detail::find_signature_impl( );

	namespace detail
	{
		struct vtable_pointer_impl
		{
			void* operator()(const std::string_view& from, const std::string_view& table_name) const;
		};

		template <typename T>
		struct vtable_pointer_fn
		{
			auto operator()(const std::string_view& from) const
			{
				const auto table_name = nstd::type_name<T, "cheat", "csgo">( );
				void*      ptr        = std::invoke(vtable_pointer_impl( ), from, table_name);
				return static_cast<T*>(ptr);
			}
		};
	}

	template <typename T>
	_INLINE_VAR constexpr auto vtable_pointer = detail::vtable_pointer_fn<T>( );
}
