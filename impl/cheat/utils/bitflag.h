#pragma once

namespace cheat::utl
{
	/*template <typename T, typename ...Ts>
	concept all_same = sizeof...(Ts) > 0 && (std::same_as<T, Ts> && ...);

	template <typename ...T>
	concept all_different = sizeof...(T) <= 1 || !all_same<T...>;*/

	template <typename A, typename B = A>
	concept bitflag_native_support = requires(A a, B b)
	{
		{ ~a }->std::_Boolean_testable;
		{ ~b }->std::_Boolean_testable;
		{ a & b }->std::_Boolean_testable;
		{ a ^ b }->std::_Boolean_testable;
		{ a | b }->std::_Boolean_testable;
	};

	/*template <typename A, typename B = A>
	concept bits_shift_supported = requires(A&& a, B&& b)
	{
		a >> b << a;
	};*/

	namespace detail
	{
		template <typename Tested, typename Current, typename ...Tnext>
		constexpr auto size_selector( )
		{
			if constexpr (sizeof(Tested) == sizeof(Current))
				return std::type_identity<Current>{ };
			else if constexpr (sizeof...(Tnext) > 0)
				return size_selector<Tested, Tnext...>( );
		}

		template <typename T>
		using signed_selector = decltype(size_selector<T, std::int8_t, std::int16_t, std::int32_t, std::int64_t>( ));
		template <typename T>
		using unsigned_selector = decltype(size_selector<T, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>( ));

		template <typename T>
		constexpr auto raw_bitflag_type( )
		{
			if constexpr (std::is_enum_v<T>)
				return std::type_identity<std::underlying_type_t<T>>{ };
			else
				return std::type_identity<T>{ };
		}

		template <typename T>
		constexpr bool _Is_bitflag_constructible( )
		{
			if constexpr (std::is_enum_v<T>)
				return true;
			else if constexpr (std::integral<T>)
				return !std::same_as<T, bool>;
			else
				return bitflag_native_support<T>;
		}

		template <typename T>
		concept bitflag_constructible = _Is_bitflag_constructible<T>( );
	}

	template <detail::bitflag_constructible T>
	class bitflag
	{
	public:
		using value_type = T;
		using raw_type = typename decltype(detail::raw_bitflag_type<T>( ))::type;

	private:
		template <typename T2>
		static constexpr raw_type To_raw_(const T2& val)
		{
			if constexpr (std::same_as<T2, bitflag>)
				return val.get_raw( );
			else
				return static_cast<raw_type>(val);
		}

		template <typename ...Args>
		static constexpr raw_type Combine_(const Args& ...args)
		{
			return (To_raw_(args) | ...);
		}

		template <typename ...Args>
		constexpr void Set_(const Args& ...args)
		{
			raw_type temp = Combine_(args...);
			flags__ = static_cast<value_type>(temp);
		}

		T flags__ = { };

	public:
		constexpr value_type get( ) const
		{
			return flags__;
		}

		constexpr raw_type get_raw( ) const
		{
			return To_raw_(flags__);
		}

		static constexpr bool native = bitflag_native_support<value_type>;

		constexpr auto convert( ) const
		{
			if constexpr (!native)
				return get_raw( );
			else
				return get( );
		}

		/* constexpr bitflag( )
			 : flags__((T)0) {}

		 constexpr bitflag(T&& val)
			 : flags__(FWD(val)) {}*/

		constexpr bitflag(const T& val)
		{
			Set_(val);
		}

		template <typename ...Args>
		constexpr bitflag(const Args& ...args)
			requires((std::convertible_to<Args, value_type> || std::convertible_to<Args, raw_type>) && ...)
		{
			if constexpr (sizeof...(Args) == 0)
				Set_(0);
			else
				Set_(args...);
		}

	private:
		template <typename T2>
		static constexpr void Validate_shift_(const T2& value)
		{
#ifdef _DEBUG
			if (static_cast<raw_type>(value) == value)
				return;
			if (std::is_constant_evaluated( ))
				throw std::logic_error("incorrect shift value!");
			else
				// ReSharper disable once CppUnreachableCode
				BOOST_ASSERT("incorrect shift value!");

#endif
		}

	public:
		constexpr bitflag shift_rignt(const raw_type& val) const
		{
			const auto value = this->convert( ) >> val;
			Validate_shift_(value);
			return value;
		}

		constexpr bitflag shift_left(const raw_type& val) const
		{
			const auto value = this->convert( ) << val;
			Validate_shift_(value);
			return value;
		}

		template <typename ...Args>
		constexpr bool has(const Args& ...args) const
		{
			auto raw = this->get_raw( );

			return ((raw & To_raw_(args)) || ...);
		}

		template <typename ...Args>
		constexpr bool has_all(const Args& ...args) const
		{
			auto raw = this->get_raw( );
			auto flags = Combine_(args...);

			return raw & flags;
		}

		template <typename ...Args>
		constexpr bitflag& add(const Args& ...args)
		{
			Set_(this->flags__, args...);
			return *this;
		}

		template <typename ...Args>
		constexpr bitflag& set(const Args& ...args)
		{
			*this = { };
			Set_(args...);
			return *this;
		}

		template <typename ...Args>
		constexpr bitflag& remove(const Args& ...args)
		{
			auto raw = this->get_raw( );
			auto flags = Combine_((args)...);

			auto result = raw & ~flags;
			Set_(result);

			return *this;
		}

		constexpr bool operator ==(const bitflag& other) const = default;
	};

	namespace detail
	{
		template <typename T, typename ...Tnext>
		constexpr auto _Get_first_enum( )
		{
			if constexpr (std::is_enum_v<T>)
				return std::type_identity<T>{ };
			else if constexpr (sizeof...(Tnext) == 0)
				return std::type_identity<void>{ };
			else
				return _Get_first_enum<Tnext...>( );
		}

		template <typename T1, typename T2, typename ...Tnext>
		constexpr auto _Biggest_type_selector( )
		{
			using type = std::conditional_t<sizeof(T1) < sizeof(T2), T2, T1>;
			if constexpr (sizeof...(Tnext) == 0)
				return std::type_identity<type>{ };
			else
				return _Biggest_type_selector<type, Tnext...>( );
		}

		template <typename T, typename ...Tnext>
		constexpr bool _Contains_different_enums( )
		{
			if constexpr (std::is_enum_v<T>)
				return ((std::is_enum_v<Tnext> && !std::same_as<T, Tnext>) || ...);
			else if constexpr (sizeof...(Tnext) == 0)
				return false;
			else
				return _Contains_different_enums<Tnext...>( );
		}
	}

	namespace detail
	{
		template <typename T = void, typename ...Args>
		constexpr bool _Is_all_same( )
		{
			return (std::same_as<T, Args> && ...);
		}

		template <typename T>
		constexpr bool _Is_unsigned( )
		{
			if constexpr (std::is_class_v<T>)
				return _Is_unsigned<typename T::value_type>( );
			else if constexpr (std::is_enum_v<T>)
				return _Is_unsigned<std::underlying_type_t<T>>( );
			else
				return std::is_unsigned_v<T>( );
		}
	}

	template <typename ...T>
	constexpr auto make_bitflag(T ...args)
	{
		using namespace detail;

		if constexpr (sizeof...(T) == 1)
		{
			return utl::bitflag<typename tuple<T...>::_This_type>(args...);
		}
		else if constexpr (_Is_all_same<T...>( ))
		{
			//select enum type or raw type

			using type = typename tuple<T...>::_This_type;
			if constexpr (_Is_bitflag_constructible<type>( ))
				return utl::bitflag<type>(args...);
			else
				static_assert(std::_Always_false<type>, "Unsupported bitflag type");
		}
		else if constexpr (_Contains_different_enums<T...>( ))
		{
			static_assert(std::_Always_false<T>, "Two or more different enums same time");
		}
		else
		{
			//select enum type or generate raw type

			using enum_t = typename decltype(_Get_first_enum<T...>( ))::type;
			if constexpr (!std::is_void_v<enum_t>)
			{
				return utl::bitflag<enum_t>(args...);
			}
			else //enum not found
			{
				using biggest_type = typename decltype(_Biggest_type_selector<T...>( ))::type;
				if constexpr ((!_Is_unsigned<T>( ) && ...))
					return utl::bitflag<typename signed_selector<biggest_type>::type>(args...);
				else
					return utl::bitflag<typename unsigned_selector<biggest_type>::type>(args...);
			}
		}
	}

	/*template <typename T>
	decltype(auto) bitflag_view(T&& val)
	{
		using value_type = std::remove_cvref_t<T>;
		using bflag_type = bitflag<value_type>;
		using raw_type=decltype(val);
		if constexpr (std::is_rvalue_reference_v<raw_type>)
			return bflag_type(val);
		else if constexpr (std::is_const_v<std::remove_reference_t<raw_type>>)
			return static_cast<const bflag_type&>(val);
		else
			return static_cast<bflag_type&>(val);
	}*/
}
