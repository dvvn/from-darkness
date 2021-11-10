#pragma once

namespace cheat::detail::netvars
{
	template <typename T>
	auto unwrap_iter(T&& itr)
	{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		return itr.operator->( );
#else
		return std::addressof(itr->second);
#endif
	}

	struct netvars_storage_iter : netvars_storage::iterator
	{
#ifndef CHEAT_NETVARS_RESOLVE_TYPE
		auto& operator*( )
		{
			return *unwrap_iter(*this);
		}
#endif

		netvars_storage_iter( ) = default;

		netvars_storage_iter(netvars_storage::iterator itr)
			: netvars_storage::iterator(itr)
		{
		}
	};

	struct netvars_root_storage_iter : netvars_root_storage::iterator
	{
#ifndef CHEAT_NETVARS_RESOLVE_TYPE
		auto& operator*( )
		{
			return *unwrap_iter(*this);
		}
#endif
		netvars_root_storage_iter( ) = default;

		netvars_root_storage_iter(netvars_root_storage::iterator itr)
			: netvars_root_storage::iterator(itr)
		{
		}
	};
}
