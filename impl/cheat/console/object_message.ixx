module;

#include <nstd/format.h>
#include <string_view>

export module cheat.console.object_message;
import cheat.tools.object_name;
import cheat.console;

template<typename Chr, typename T>
decltype(auto) convert_or_forward(const T& str)
{
	if constexpr (std::same_as<T::value_type, Chr>)
	{
		return str;
	}
	else
	{
		std::basic_string<Chr> buffer;
		buffer.assign(str.begin( ), str.end( ));
		return buffer;
	}
}

export namespace cheat::console
{
	template<typename T>
	class object_message
	{
	protected:
		template<typename Str, typename ...Args>
		void message(const Str& str, Args&&...args)const
		{
			if (disabled( ))
				return;

			using char_t = std::remove_cvref_t<decltype(str[0])>;

			const auto name = _Name( );
			const std::basic_string_view<char_t> strv = str;

			if constexpr (sizeof...(Args) == 0)
			{
				std::basic_string<char_t> buffer;

				buffer.reserve(name.size( ) + 2 + strv.size( ));
				buffer.append(name.begin( ), name.end( ));
				buffer += ':';
				buffer += ' ';
				buffer.append(strv);
				log(std::move(buffer));
			}
			else
			{
				constexpr std::string_view basic_hint = "{}: ";
				std::basic_string<char_t> hint;
				hint.reserve(basic_hint.size( ) + strv.size( ));
				hint.append(basic_hint.begin( ), basic_hint.end( )).append(strv);
				log(hint, convert_or_forward<char_t>(name), std::forward<Args>(args)...);
			}
		}

		object_message( )
		{
			message("created");
		}

		~object_message( )
		{
			message("destroyed");
		}

	private:
		std::string_view _Name( )const
		{
			return tools::object_name<T>( );
		}
	};
}