#pragma once

#include <ww898/utf_converters.hpp>

#include <string>

template < >
struct ww898::utf::detail::utf_selector<char8_t> final
{
	using type = utf8;
};

// ReSharper disable CppRedundantInlineSpecifier
namespace nstd
{
	namespace detail
	{
		template <typename Test, typename T>
		struct size_checker : std::bool_constant<sizeof(Test) == sizeof(T)>
		{
			using type = T;
		};

		template <typename T, typename ...Ts>
		using type_selector_t = typename std::disjunction<size_checker<T, Ts>...>::type;

		template <typename CharType, typename T>
		concept unistring_valid = sizeof(CharType) == sizeof(T) && std::destructible<ww898::utf::detail::utf_selector<CharType>>;

		template <typename T>
		concept has_array_access = requires(const T& obj)
		{
			obj[0];
		};

		template <typename T>
		concept std_string_based_impl = requires
		{
			typename T::value_type;
			typename T::traits_type;
			typename T::allocator_type;
		} && std::convertible_to<T, std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>>;

		template <typename T>
		concept std_string_based = std_string_based_impl<std::remove_cvref_t<T>>;

		template <typename T
				, unistring_valid<T> CharType = type_selector_t<T, char8_t, char16_t, char32_t> //--
				, typename Tr = std::char_traits<CharType>                                      //--
				, typename Al=std::allocator<CharType>>
		class unistring_base : public std::basic_string<CharType, Tr, Al>
		{
		public:
			_CONSTEXPR20_CONTAINER unistring_base() = default;

			template <typename Chr>
			_CONSTEXPR20_CONTAINER void uni_assign(const std::basic_string_view<Chr>& in)
			{
				if constexpr (sizeof(Chr) == sizeof(T))
				{
					this->resize(in.size( ));
					std::copy(in._Unchecked_begin( ), in._Unchecked_end( ), this->_Unchecked_begin( ));
				}
				else
					this->uni_assign(ww898::utf::conv<CharType>(in));
			}

			/*template <typename Chr>
			_CONSTEXPR20_CONTAINER void uni_assign(std::basic_string<Chr>&& in)
			{
				if constexpr (std::same_as<Chr, CharType>)
					std::swap(in, *this);
				else
					this->uni_assign(std::basic_string_view<Chr>(in));
			}*/

			template <std_string_based Q>
			_CONSTEXPR20_CONTAINER void uni_assign(Q&& str)
			{
				using V = std::remove_cvref_t<Q>;
				using char_type = typename V::value_type;
				using traits_type = typename V::traits_type;
				using allocator_type = typename V::allocator_type;

				using str_type = std::basic_string<char_type, traits_type, allocator_type>;
				using view_type = std::basic_string_view<char_type, traits_type>;

				if constexpr (!std::is_rvalue_reference_v<decltype(str)>
							  || !std::same_as<char_type, CharType>
							  || !std::same_as<traits_type, Tr>
							  || !std::same_as<allocator_type, Al>)
				{
					this->uni_assign(view_type(str));
				}
				else
				{
					auto& out = static_cast<str_type&>(*this);
					auto& in  = static_cast<str_type&>(str);
					out.assign(std::move(in));
				}
			}

			template <typename Chr, size_t N>
			_CONSTEXPR20_CONTAINER void uni_assign(const Chr (&str)[N])
			{
#if _CONTAINER_DEBUG_LEVEL > 0
				_STL_ASSERT(Tr::length(str) == N - 1, "string contains unsupported character inside!");
#endif
				this->uni_assign(std::basic_string_view<Chr>(str, std::next(str, N - 1)));
			}

			template <typename Q>
			_CONSTEXPR20_CONTAINER unistring_base& uni_append(const Q& str)
			{
				if constexpr (!has_array_access<Q>)
				{
					static_assert(!std::is_class_v<Q>,__FUNCTION__": unsipported type");
					const Q fake_string[] = {str, static_cast<Q>('\0')};
					return uni_append(fake_string);
				}
				else
				{
					using char_type = std::remove_cvref_t<decltype(str[0])>;
					// ReSharper disable CppInconsistentNaming
					auto _Size = [&]
					{
						if constexpr (std::is_class_v<Q>)
							return str.size( );
						else if constexpr (!std::is_bounded_array_v<Q>)
							return Tr::length(str);
						else
						{
							constexpr auto val = sizeof(Q) / sizeof(char_type);
#if _CONTAINER_DEBUG_LEVEL > 0
								_STL_ASSERT(Tr::length(str) == val-1, "string contains unsupported character inside!");
#endif
							return val - 1;
						}
					}( );
					auto _Begin = std::addressof(str[0]);
					auto _End   = _Begin + _Size;

					// ReSharper restore CppInconsistentNaming

					if constexpr (sizeof(char_type) == sizeof(T))
					{
						this->append(_Begin, _End);
						return *this;
					}
					else
					{
						using ww898::utf::utf_selector_t;
						ww898::utf::conv<utf_selector_t<char_type>, utf_selector_t<CharType>>(_Begin, _End, std::back_inserter(*this));
						return *this;
					}
				}
			}
		};
	}

	template <typename T>
	class unistring : public detail::unistring_base<T>
	{
	public:
		_CONSTEXPR20_CONTAINER unistring() = default;

		template <typename S>
		_CONSTEXPR20_CONTAINER unistring(S&& str)
		{
			this->uni_assign(std::forward<S>(str));
		}
	};
}
