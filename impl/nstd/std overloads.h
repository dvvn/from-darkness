#pragma once

namespace nstd
{
	template <typename T>
	concept string_viewable = requires(const T& obj)
	{
		obj.view( );
	};
	template <typename T>
	using string_viewable_t = decltype(std::declval<T>( ).view( ));

	template <typename Stream, typename T>
	concept stream_possible = requires(Stream stream, const T& obj)
	{
		stream << obj;
	};

	namespace detail
	{
		template <typename T, typename Formatter = std::formatter<string_viewable_t<T>>>
		struct formatter_string_viewable: Formatter
		{
			template <class FormatContext>
			typename FormatContext::iterator format(const T& val, FormatContext& ctx)
			{
				return Formatter::format(val.view( ), ctx);
			}
		};
	}
}

namespace std
{
	template <nstd::string_viewable T>
	struct formatter<T>: nstd::detail::formatter_string_viewable<T>
	{
	};

	template <typename E, typename Tr, nstd::string_viewable T>
		requires(sizeof(E) >= sizeof(typename nstd::string_viewable_t<T>::value_type))
	basic_ostream<E, Tr>& operator<<(basic_ostream<E, Tr>& s, const T& val)
	{
		return s << val.view( );
	}

	template <typename E, typename Tr, nstd::string_viewable T>
		requires(sizeof(E) >= sizeof(typename nstd::string_viewable_t<T>::value_type))
	basic_ostream<E, Tr>&& operator<<(basic_ostream<E, Tr>&& s, const T& val)
	{
		s << val;
		return move(s);
	}
}
