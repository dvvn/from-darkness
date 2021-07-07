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

		static reference get( )
		{
			static_assert(std::is_default_constructible_v<T>, "T must be default constructible!");
			static T cache = T( );
			return cache;
		}

		static pointer get_ptr( )
		{
			return addressof(get( ));
		}

		reference       operator*( ) { return get( ); }
		const_reference operator*( ) const { return get( ); }
		pointer         operator->( ) { return get_ptr( ); }
		const_pointer   operator->( ) const { return get_ptr( ); }

		//constexpr auto operator()( ) -> reference { return get( ); }
		//constexpr auto operator()( ) const -> const_reference { return get( ); }

		explicit operator reference( ) { return get( ); }
		explicit operator const_reference( ) const { return get( ); }
	};

	template <typename T, size_t Count = 0>
	class one_instance_shared
	{
	public:
		using element_type = T;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;
		using stored_type = weak_ptr<T>;
		using shared_type = shared_ptr<T>;

		static shared_type get_shared( )
		{
#if 0
			if (shared_instance__.has_value( ) == false)
			{
				
				shared_type ptr = make_shared<T>( );
				shared_instance__.emplace(ptr);
				return ptr;
			}
			return (*shared_instance__).lock( );
#endif

			static_assert(std::is_default_constructible_v<T>, __FUNCTION__": T must be default constructible!");

			static shared_ptr<T> shared = make_shared<T>( );

			shared_ptr<T> weak_shared;
			if (shared)
				weak_shared = move(shared);

			static weak_ptr<T> weak = weak_shared;
			return weak.lock( );
		}

		static reference get( )
		{
			return *get_shared( );
		}

		static pointer get_ptr( )
		{
			return get_shared( ).get( );
		}

		//private:
		//inline static optional<weak_ptr<T>> shared_instance__;
	};
}
