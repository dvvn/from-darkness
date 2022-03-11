module;

#include <nstd/format.h>
#include <string>
#include <functional>

export module cheat.console.object_message;
import cheat.tools.object_name;
import cheat.console;

template<typename Chr, typename T>
auto try_convert(const T& str)
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

template<typename Str, typename NameGetter, typename ...Args>
void console_log(const Str& str, NameGetter name_getter, Args&&...args)
{
	using char_t = std::remove_cvref_t<decltype(str[0])>;

	if constexpr (sizeof...(Args) == 0)
	{
		cheat::console::log([&]
		{
			const auto name = std::invoke(name_getter);
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
		const auto hint_fn = [&]
		{
			const std::basic_string_view<char_t> strv = str;
			constexpr std::string_view basic_hint = "{}: ";

			std::basic_string<char_t> hint;
			hint.reserve(basic_hint.size( ) + strv.size( ));
			hint.append(basic_hint.begin( ), basic_hint.end( )).append(strv);
			return hint;
		};

		const auto name_fn = [&]
		{
			return try_convert<char_t>(std::invoke(name_getter));
		};

		log(hint_fn, name_fn, std::forward<Args>(args)...);
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
			console_log(str, [&] {return _Name( ); }, std::forward<Args>(args)...);
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