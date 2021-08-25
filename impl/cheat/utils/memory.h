#pragma once
#include "cheat/sdk/IAppSystem.hpp"

namespace cheat
{
	class csgo_interfaces;
}

namespace cheat::utils
{
	struct find_signature_impl
	{
		nstd::address operator()(const nstd::memory_block& from, const std::string_view& sig) const;
		//nstd::address operator()(const std::string_view& dll_name, const std::string_view& sig) const=delete;
	};

	[[deprecated]]
	_INLINE_VAR constexpr auto find_signature = find_signature_impl( );

	struct vtable_pointer_impl
	{
		void* operator()(const std::string_view& from, const std::string_view& table_name) const = delete;
		void* operator()(nstd::os::module_info& info, const std::string_view& table_name) const;
	};

	template <typename T>
	struct vtable_pointer_fn
	{
		[[deprecated]]
		auto operator()(const std::string_view& from) const
		{
			constexpr auto table_name = nstd::type_name<T, "cheat", "csgo">( );
			void*          ptr        = std::invoke(vtable_pointer_impl( ), from, table_name);
			return static_cast<T*>(ptr);
		}
	};

	template <typename T>
	[[deprecated]]
	_INLINE_VAR constexpr auto vtable_pointer = vtable_pointer_fn<T>( );

	struct csgo_interfaces_cache_impl
	{
		using entry_type = nstd::unordered_map<std::string_view, csgo::InstantiateInterfaceFn>;
		using cache_type = nstd::ordered_map<nstd::os::module_info*, entry_type>;

		nstd::address operator()(nstd::os::module_info* target_module, const std::string_view& interface_name);

	private:
		cache_type cache__;
	};
}
