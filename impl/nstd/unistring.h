#pragma once

#include <ww898/utf_converters.hpp>

#include <string>

namespace ww898::utf::detail
{
	template < >
	struct utf_selector<char8_t> final
	{
		using type = utf8;
	};
}

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

		template <typename T, unistring_valid<T> CharType = type_selector_t<T, char8_t, char16_t, char32_t>, typename Tr = std::char_traits<CharType>>
		class unistring_base : public std::basic_string<CharType, Tr>
		{
		public:
			// ReSharper disable CppInconsistentNaming
			auto& _Uni_unwrap()
			{
				return *static_cast<std::basic_string<CharType, Tr>*>(this);
			}

			std::basic_string_view<CharType, Tr> _Uni_unwrap() const
			{
				return (std::_Const_cast(this)->_Uni_unwrap( ));
			}

			// ReSharper restore  CppInconsistentNaming

			_CONSTEXPR20_CONTAINER unistring_base() = default;

			/*template <typename T1,typename Chr1,typename Tr1>
			_CONSTEXPR20_CONTAINER void uni_assign(const unistring_base<T1,Chr1,Tr1>&str)
			{
				uni_assign(str._Uni_unwrap());
			}
			template <typename T1,typename Chr1,typename Tr1>
			_CONSTEXPR20_CONTAINER void uni_assign(unistring_base<T1,Chr1,Tr1>&&str)
			{
				uni_assign(std::move(str._Uni_unwrap()));
			}*/

			template <typename Chr>
			_CONSTEXPR20_CONTAINER void uni_assign(const std::basic_string_view<Chr>& in)
			{
				if constexpr (sizeof(Chr) == sizeof(T))
				{
					this->resize(in.size( ));
					std::copy(in._Unchecked_begin( ), in._Unchecked_end( ), this->_Unchecked_begin( ));
				}
				else
					this->_Uni_unwrap( ) = ww898::utf::conv<CharType>(in);
			}

			template <typename Chr>
			_CONSTEXPR20_CONTAINER void uni_assign(std::basic_string<Chr>&& in)
			{
				if constexpr (std::same_as<Chr, CharType>)
					std::swap(in, *this);
				else
					this->uni_assign(std::basic_string_view<Chr>(in));
			}

			template <typename Chr, size_t N>
			_CONSTEXPR20_CONTAINER void uni_assign(const Chr (&str)[N])
			{
#if _CONTAINER_DEBUG_LEVEL > 0
				_STL_ASSERT(Tr::length(str) == N - 1, "string contains unsupported character inside!");
#endif
				this->uni_assign(std::basic_string_view<Chr>(str, std::next(str, N - 1)));
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
