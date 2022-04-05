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
		void operator()(const Str& str, Args&&...args) const
		{
			using char_t = std::remove_cvref_t<decltype(str[0])>;

			if constexpr (sizeof...(Args) == 0)
			{
				console::log([&]
				{
					const auto name = this->get_name( );
					const std::basic_string_view<char_t> strv = str;

					std::basic_string<char_t> buffer;
					buffer.reserve(name.size( ) + 2 + strv.size( ));
					buffer.append(name.begin( ), name.end( ));
					buffer += ':';
					buffer += ' ';
					buffer.append(strv);

					return buffer;
				});
			}
			else
			{
				console::log([&]
				{
					const std::basic_string_view<char_t> strv = str;
					constexpr std::string_view basic_hint = "{}: ";

					std::basic_string<char_t> hint;
					hint.reserve(basic_hint.size( ) + strv.size( ));
					hint.append(basic_hint.begin( ), basic_hint.end( ));
					hint.append(strv);
					return hint;
				}, []
				{
					const auto name = this->get_name( );
					if constexpr (std::same_as<char_t, char>)
					{
						return name;
					}
					else
					{
						std::basic_string<char_t> buffer;
						buffer.reserve(name.size( ));
						buffer.assign(name.begin( ), name.end( ));
						return buffer;
					}
				}, std::forward<Args>(args)...);
			}
		}

		std::string_view get_name( ) const
		{
			return tools::object_name<T>( );
		}
	};

	template<class T>
	inline constexpr auto object_message = []<typename ...Args>(Args&&...args)
	{
		constexpr object_message_impl<T> impl;
		impl(std::forward<Args>(args)...);
	};

#define OBJECT_MSG(_MSG_)\
	template<class T>\
	inline constexpr auto object_##_MSG_ = []\
	{\
		object_message<T>(#_MSG_);\
	};\

	OBJECT_MSG(created);
	OBJECT_MSG(destroyed);
	OBJECT_MSG(found);
	/*OBJECT_MSG(loaded);
	OBJECT_MSG(hooked);
	OBJECT_MSG(enabled);
	OBJECT_MSG(disabled);*/

	template<class T>
	struct object_message_auto
	{
		object_message_auto( )
		{
			object_created<T>( );
		}

		~object_message_auto( )
		{
			object_destroyed<T>( );
		}

		template<typename ...Args>
		void message(Args&& ...args)const
		{
			object_message<T>(std::forward<Args>(args)...);
		}
	};
}