module;

#include <cheat/gui2/type_info.h>

#include <concepts>

export module cheat.gui2.type_info;
#ifndef RTTI_ENABLED
import nstd.type_name;
#endif

export namespace cheat::gui2
{
	template<class T>
	TYPE_INFO_CONSTEXPR type_info_t get_type_info(const std::type_identity<T> = {}) noexcept
	{
		return
#ifdef RTTI_ENABLED
			typeid(T)
#else
			nstd::type_name<T>( )
#endif
			;
	}

	template<class T>
	TYPE_INFO_CONSTEXPR type_info_t get_type_info(const T&) noexcept
	{
		return get_type_info(std::type_identity<T>( ));
	}

	template<class T>
	concept type_info_compatible = requires(const T & obj)
	{
		{
			obj.type( )
		}->std::same_as<type_info_t>;
	};

	class type_info
	{
	public:
		virtual ~type_info( );
		virtual type_info_t type( ) const noexcept = 0;

		bool operator==(const type_info& other) const noexcept;

		template<type_info_compatible T>
		bool operator==(const T& other) const noexcept
		{
			return this->type( ) == other.type( );
		}
	};

	template<class T>
	class type_info_for : public virtual type_info
	{
	public:
		type_info_t type( ) const noexcept final
		{
			return get_type_info<T>( );
		}

		TYPE_INFO_CONSTEXPR bool operator==(const type_info_for& other) const noexcept
		{
			return true;
		}

		template<typename T1>
		TYPE_INFO_CONSTEXPR bool operator==(const type_info_for<T1>& other) const noexcept
		{
			return get_type_info<T>( ) == get_type_info<T1>( );
		}
	};
}