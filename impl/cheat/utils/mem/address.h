#pragma once

namespace cheat::utl::mem
{
	// class size is only 4 bytes on x86-32 and 8 bytes on x86-64.
	class address
	{
		uintptr_t value__;

		void Error_handler_( ) const;

	public:
		address( ) : value__(0)
		{
		}

		address(uintptr_t a) : value__(a)
		{
		}

		explicit address(std::nullptr_t) : address( )
		{
		}

		address(const void* a) : address(reinterpret_cast<uintptr_t>(a))
		{
		}

		constexpr auto operator<=>(const address& other) const = default;

		uintptr_t value( ) const;

		/// @brief cast / add offset and cast.
		template <typename T>
		T cast( ) const
		{
			Error_handler_( );
			return (T)value__;
		}

		template <typename T = uintptr_t>
		T* raw( ) const
		{
			return cast<T*>( );
		}

		template <typename T = uintptr_t>
		const T& ref( ) const
		{
			return *raw<T>( );
		}

		template <typename T = uintptr_t>
		T& ref( )
		{
			return *raw<T>( );
		}

		address operator*( ) const;

		//derefference
		address deref(size_t count) const;
		address deref_safe(size_t count) const;

		//--------------------
		address& operator+=(const address& offset);
		address  operator+(const address& offset) const;
		address  operator-(const address& offset) const;
		address& operator-=(const address& offset);

		address add(const address& offset) const;
		address remove(const address& offset) const;

		//--------------------------

		// follow relative8 and relative16/32 offsets.
		address rel8(size_t offset) const;
		address rel32(size_t offset) const;
	};
}
