module;

#include <nstd/format.h>

#include <string>

export module cheat.console.object_message;
import cheat.tools.object_name;
import cheat.console;

export namespace cheat::console
{
	template<class T>
	struct object_message_impl
	{
		template<typename Str, typename ...Args>
		void operator()(const Str& str, Args&&...args) const noexcept
		{
			log([&]( ) noexcept
			{
				const auto name = this->object_name( );

				using char_t = std::remove_cvref_t<decltype(str[0])>;
				const std::basic_string_view<char_t> strv = str;
				std::basic_string<char_t> buffer;

				buffer.reserve(name.size( ) + 2 + strv.size( ));
				buffer.append(name.begin( ), name.end( ));
				buffer += ':';
				buffer += ' ';
				buffer.append(strv);

				return buffer;
			}, std::forward<Args>(args)...);
		}

		std::string_view object_name( ) const noexcept
		{
			return cheat::tools::object_name<T>;
		}
	};

	template<class T>
	constexpr object_message_impl<T> object_message;

	template<class T>
	struct object_message_auto final : object_message_impl<T>
	{
		object_message_auto( )
		{
			std::invoke(*this, "created");
		}

		~object_message_auto( )
		{
			std::invoke(*this, "destroyed");
		}
	};
}