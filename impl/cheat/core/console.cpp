#include "console.h"

using namespace cheat;
using namespace utl;

static string _Get_time_str( )
{
	static auto num_to_char = [](int num) -> char
	{
#define CONVERT(x)\
	case x: return #x[0]

		switch (num)
		{
			CONVERT(0);
			CONVERT(1);
			CONVERT(2);
			CONVERT(3);
			CONVERT(4);
			CONVERT(5);
			CONVERT(6);
			CONVERT(7);
			CONVERT(8);
			CONVERT(9);
			default:
				BOOST_ASSERT("Unsupported number");
				return 0;
		}

#undef CONVERT
	};
	static auto add_zero = [](int i)-> string
	{
		string str;
		if (i < 10)
		{
			str += '0';
			str += num_to_char(i);
		}
		else
		{
			str += num_to_char(i / 10);
			str += num_to_char(i % 10);
		}

		return str;

		/*if (i >= 10)
			return to_string(i);
		return '0' + to_string(i);*/
	};

	const auto now = chrono::system_clock::now( );
#if 1
	const auto time = chrono::system_clock::to_time_t(now);

	tm timeinfo;
	const auto result = localtime_s(&timeinfo, &time);
	(void)result;
	BOOST_ASSERT(result == 0);

	return format("[{}:{}:{}] ", timeinfo.tm_hour, add_zero(timeinfo.tm_min), add_zero(timeinfo.tm_sec));

#else


	auto duration = now.time_since_epoch( );
	using Days = chrono::duration<int, boost::ratio_multiply<chrono::hours::period, boost::ratio<8>
						  >>; /* UTC: +8:00 */

	const auto days_val = duration_cast<Days>(duration);
	duration -= days_val;
	const auto hours_val = duration_cast<chrono::hours>(duration);
	duration -= hours_val;
	const auto minutes_val = duration_cast<chrono::minutes>(duration);
	duration -= minutes_val;
	const auto seconds_val = duration_cast<chrono::seconds>(duration);
	duration -= seconds_val;
	const auto milliseconds_val = duration_cast<chrono::milliseconds>(duration);
	//duration -= milliseconds_val;
	//const auto microseconds_val = duration_cast<chrono::microseconds>(duration);
	//duration -= microseconds_val;
	//const auto nanoseconds_val = duration_cast<chrono::nanoseconds>(duration);

	return format("[{}:{}:{}:{}] ", hours_val.count( ), add_zero(minutes_val.count( )), add_zero(seconds_val.count( )), add_zero(milliseconds_val.count( )));
#endif
}

console::~console( )
{
	if (write_redirected__)
	{
		fclose(write__);
	}
	if (console_allocated__)
	{
		FreeConsole( );
		PostMessage(console_window__, WM_CLOSE, 0U, 0L);
	}
}

console::console( ) //: sem__(1)
{
	//load_synchronously_ = true;
}

void console::Load( )
{
#ifdef CHEAT_HAVE_CONSOLE
	console_window__ = GetConsoleWindow( );
	if (console_window__ != nullptr)
	{
		write__ = stdout;
		write_redirected__ = false;
		console_allocated__ = false;
	}
	else
	{
		//create new console window
		if (!AllocConsole( ))
			BOOST_ASSERT("Unable to alloc console!");
		else
			console_allocated__ = true;

#if 0
		//attach to already exsisting console window
		if(AttachConsole(ATTACH_PARENT_PROCESS) == 0)
			BOOST_ASSERT("Unable to attach console!");

#endif

		//redirect cout stdin - to read / stdout - to write in console window
		//no need to create a backup of stdout
		FILE* write;
		if (freopen_s(&write, "CONOUT$", "w", stdout) != 0)
			BOOST_ASSERT("Unable to open stream");
		else
		{
			write__ = write;
			write_redirected__ = true;
		}

		const auto full_path = /*basic_string*/all_modules::get( ).current( ).full_path( );
		//ranges::replace(full_path, '\\', '/');
		(void)full_path;

		if (!SetConsoleTitle(full_path.data( )))
			BOOST_ASSERT("Unable set console title");

		console_window__ = GetConsoleWindow( );
		BOOST_ASSERT_MSG(console_window__ != nullptr, "Unable to get console window");
	}
#endif
}

string console::Get_loaded_message( ) const
{
#ifdef CHEAT_HAVE_CONSOLE
	return service_shared::Get_loaded_message( );
#else
	return this->Get_loaded_message_disabled( );
#endif
}

void console::Wait_for_write_( ) const
{
	//sometimes it still nullptr after init, idk why
	while (!write__)
	{
		this_thread::sleep_for(chrono::milliseconds(10));
	}
}

#if 0
class write_helper
{
public:
	template <typename ...Ts>
	using binder = std::_Front_binder<std::decay_t<Ts>...>;

	using caller_type = variant<
		binder<decltype(_Write_char), char>, binder<decltype(_Write_string), string>>;

	write_helper(string&& data): caller__(data.size( ) == 1
											  ? caller_type(std::in_place_index<0>, bind_front(_Write_char, data[0]))
											  : caller_type(std::in_place_index<1>, bind_front(_Write_string, move(data))))
	{
	}

	write_helper(const string_view& data): caller__(data.size( ) == 1
														? caller_type(std::in_place_index<0>, bind_front(_Write_char, data[0]))
														: caller_type(std::in_place_index<1>, std::bind_front(_Write_string, string(data))))
	{
	}

	auto operator()(FILE*& file) const -> void
	{
		switch (caller__.index( ))
		{
			case 0:
				invoke(get<0>(caller__), file);
				break;
			case 1:
				invoke(get<1>(caller__), file);
				break;
			default:
				BOOST_ASSERT("Unknown index");
		}
	}

private:
	caller_type caller__;
};
#endif

void console::write(const string_view& str) const
{
	if (str.size( ) == 1)
		write_char(str[0]);
	else
	{
		BOOST_ASSERT(str.size( ) > 1);
		Wait_for_write_( );
		[[maybe_unused]] const auto written = std::fwrite(str.data( ), sizeof(string_view::value_type), str.size( ), write__);
		BOOST_ASSERT(written == str.size( ));
	}
}

void console::write_time( ) const
{
	write(_Get_time_str( ));
}

#if 0
class write_line_helper: write_helper
{
public:
	write_line_helper(string&& data) : write_helper(data)
	{
	}

	write_line_helper(const string_view& data) : write_helper(data)
	{
	}

	auto operator()(FILE*& file) const -> void
	{
		_Write_string(time__, file);
		write_helper::operator()(file);
		_Write_char('\n', file);
	}

private:
	string time__ = _Get_time_str( );
};
#endif

void console::write_line(const string_view& str) const
{
	if constexpr (/*!sync*/false)
	{
		write_time( );
		write(str);
		write_char('\n');
	}
	else
	{
		auto time = _Get_time_str( );
		string str1;
		str1.reserve(time.size( ) + str.size( ) + 1);
		str1 += (move(time));
		str1 += (str);
		str1 += ('\n');
		write(str1);
	}
}

void console::write_char(char c) const
{
	Wait_for_write_( );
	const auto written = std::fputc(c, write__);
	(void)written;
	BOOST_ASSERT(written == c);
}
