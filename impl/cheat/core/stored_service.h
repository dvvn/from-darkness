#pragma once

#include <memory>
#include <concepts>

namespace cheat
{
	class basic_service;

	template <typename T = basic_service>
	struct stored_service : std::shared_ptr<T>
	{
		static_assert(std::derived_from<T, basic_service>);

		stored_service( ) = default;

		template <class Q>
		stored_service(Q&& basic_service)
			: std::shared_ptr<T>((std::forward<Q>(basic_service)))
		{
		}
	};
}

//namespace std
//{
//	template <typename To, typename From>
//	cheat::stored_service<To> dynamic_pointer_cast(cheat::stored_service<From>&& ptr)
//	{
//		return std::dynamic_pointer_cast<To>(static_cast<std::shared_ptr<From>&&>(ptr));
//	}
//
//	template <typename To, typename From>
//	cheat::stored_service<To> dynamic_pointer_cast(const cheat::stored_service<From>& ptr)
//	{
//		return std::dynamic_pointer_cast<To>(static_cast<const std::shared_ptr<From>&>(ptr));
//	}
//}
