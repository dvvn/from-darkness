#pragma once

namespace cheat::detail::netvars
{
	template <typename T>
	struct iterator_wrapper : T
	{
		using iter_type = T;

		iterator_wrapper( ) = default;

		template <typename ...Args>
		iterator_wrapper(Args&&...args)
			: T(std::forward<Args>(args)...)
		{
		}

#ifndef CHEAT_NETVARS_RESOLVE_TYPE
		auto& operator*( ) { return T::operator->( )->second; }
		auto& operator*( ) const { return T::operator->( )->second; }
		auto operator->( ) { return std::addressof(T::operator->( )->second); }
		auto operator->( ) const { return std::addressof(T::operator->( )->second); }
#endif
	};

	template <typename T>
	iterator_wrapper(T&&) -> iterator_wrapper<std::remove_cvref_t<T>>;
}
