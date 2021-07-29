// ReSharper disable CppRedundantElseKeywordInsideCompoundStatement
#pragma once

namespace cheat::utl
{
	template <typename T, size_t Count = 0>
	class one_instance
	{
	public:
		constexpr one_instance( ) = default;

		using value_type = T;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;

		/*static reference get( )
		{
			static_assert(std::is_default_constructible_v<T>, "T must be default constructible!");
			static T cache = T( );
			return cache;
		}*/

		static pointer get_ptr( )
		{
			static_assert(std::is_default_constructible_v<T>, "T must be default constructible!");
			static T cache = T( );
			return addressof(cache);
		}
	};

	template <typename T, size_t Count = 0>
	class one_instance_shared
	{
	public:
		using value_type = T;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;
		using weak_type = weak_ptr<T>;
		using shared_type = shared_ptr<T>;

		static shared_type get_ptr( )
		{
			static_assert(std::is_default_constructible_v<T>, __FUNCTION__": T must be default constructible!");

			static shared_type shared = make_shared<T>( );
			static weak_type weak;

			if (!shared)
			{
				return weak.lock( );
			}
			else
			{
				weak = shared;
				return move(shared);
			}
		}

		static weak_type get_ptr_weak( )
		{
			static weak_type weak = get_ptr( );
			return weak;
		}
	};
}
