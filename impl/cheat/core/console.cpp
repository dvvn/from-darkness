#include "console.h"

#include <corecrt_io.h>
#include <fcntl.h>

using namespace cheat;
using namespace detail;
using namespace utl;

template <typename T>
static auto _Get_time_str( )
{
	using namespace std::chrono;
	using clock = system_clock;

	const auto current_time_point = clock::now( );
	const auto current_time       = clock::to_time_t(current_time_point);
	auto       current_localtime  = tm( );

	localtime_s(&current_localtime, &current_time);

	const auto current_time_since_epoch = current_time_point.time_since_epoch( );
	const auto current_milliseconds     = duration_cast<milliseconds>(current_time_since_epoch).count( ) % 1000;
	const auto current_microseconds     = duration_cast<microseconds>(current_time_since_epoch).count( ) % 1000;

	// ReSharper disable CppInconsistentNaming
	constexpr T ZERO  = '0';
	constexpr T SPACE = ' ';
	constexpr T DOT   = '.';

	constexpr T FMT_ARG[] = {'%', 'T', '\0'};
	// ReSharper restore CppInconsistentNaming

	auto stream = std::basic_ostringstream<T>( );
	stream
			<< std::setfill(ZERO)
			<< std::put_time(&current_localtime, FMT_ARG)
			<< SPACE
			<< std::setw(3) << current_milliseconds
			<< DOT
			<< std::setw(3) << current_microseconds;
	return stream;
}

void console_data::write_cache( )
{
	while (!cache__.empty( ))
	{
		std::invoke(cache__.front( ), write__);
		cache__.pop( );
	}
}

template <bool Assert = true>
static auto _Set_mode(FILE* file, int mode)
{
	const auto old_mode = _setmode(_fileno(file), mode);
	if constexpr (Assert)
		runtime_assert(old_mode!=-1, "Unable to change mode");
	return old_mode;
}

console::~console( )
{
	if (original_mode.has_value( ))
	{
		_Set_mode<false>(write__, *original_mode);
	}
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

bool console::load_impl( )
{
	console_window__ = GetConsoleWindow( );
	if (console_window__ != nullptr)
	{
		write__             = stdout;
		write_redirected__  = false;
		console_allocated__ = false;
	}
	else
	{
		//create new console window
		if (!AllocConsole( ))
		{
			runtime_assert("Unable to alloc console!");
			return false;
		}
		console_allocated__ = true;

#if 0
		//attach to already exsisting console window
		if(AttachConsole(ATTACH_PARENT_PROCESS) == 0)
			runtime_assert("Unable to attach console!");

#endif

		//redirect cout stdin - to read / stdout - to write in console window
		//no need to create a backup of stdout
		FILE* write;
		if (freopen_s(&write, "CONOUT$", "w", stdout) != 0)
		{
			runtime_assert("Unable to open stream");
			return false;
		}
		write__            = write;
		write_redirected__ = true;

		const auto full_path = /*basic_string*/all_modules::get_ptr( )->current( ).full_path( );
		//ranges::replace(full_path, '\\', '/');
		(void)full_path;

		if (!SetConsoleTitle(full_path.data( )))
		{
			runtime_assert("Unable set console title");
			return false;
		}

		console_window__ = GetConsoleWindow( );
		if (console_window__ == nullptr)
		{
			runtime_assert("Unable to get console window");
			return false;
		}
	}

	auto prev_mode = _Set_mode<false>(write__, _O_TEXT);
	if (prev_mode == -1)
	{
		runtime_assert("Unable to set mode to UTF16");
		return false;
	}
	original_mode = prev_mode;

	return true;
}

static auto _Write_line = []<typename T>(T&& text, FILE* file)
{
	// ReSharper disable CppInconsistentNaming
	decltype(auto) _Text = nstd::as_string(std::forward<T>(text));
	using value_type = typename std::remove_cvref_t<decltype(_Text)>::value_type;

	if constexpr (std::same_as<value_type, char>)
	{
		runtime_assert(!IsTextUnicode(_Text.data( ), _Text.size( ), nullptr));
		_Set_mode(file, _O_TEXT);
	}
	else if constexpr (std::same_as<value_type, wchar_t>)
	{
		_Set_mode(file, _O_U16TEXT);
	}
	else
	{
		static_assert(std::_Always_false<value_type>,__FUNCSIG__);
		return;
	}

	[[maybe_unused]] const auto written = std::fwrite(_Text.data( ), sizeof(value_type), _Text.size( ), file);
	runtime_assert(written == _Text.size());
};

template <bool CheckSize = true, typename T>
static void _Write_line_helper(T&& text, console* instance, console_data* data)
{
	// ReSharper disable once CppInconsistentNaming
	auto&& _Text = nstd::as_string(std::forward<T>(text));

	if constexpr (CheckSize)
	{
		runtime_assert(!_Text.empty( ), "String is empty!");
		if (_Text.size( ) == 1)
			return instance->write(_Text.front( ));
	}

	const auto lock = std::scoped_lock(data->lock__);
	if (!instance->state( ).done( ))
	{
		data->cache__.push(std::bind_front(_Write_line, std::basic_string(std::forward<decltype(_Text)>(_Text))));
	}
	else
	{
		data->write_cache( );
		_Write_line(std::forward<decltype(_Text)>(_Text), data->write__);
	}
}

void console::write(std::string&& str)
{
	_Write_line_helper(std::move(str), this, this);
}

void console::write(const std::string_view& str)
{
	_Write_line_helper(str, this, this);
}

void console::write_line(const std::string_view& str)
{
	runtime_assert(!str.empty( ), "String is empty!");

	auto stream = _Get_time_str<char>( ) << " - " << str << '\n';
	_Write_line_helper<false>(std::move(stream), this, this);
}

void console::write(char c)
{
	const auto lock = std::scoped_lock(lock__);

	constexpr auto write_fn = [](const char chr, FILE* file)
	{
		_Set_mode(file, _O_TEXT);
		[[maybe_unused]] const auto written = std::fputc(chr, file);
		runtime_assert(written == chr);
	};

	if (!this->state( ).done( ))
		cache__.push(std::bind_front(write_fn, c));
	else
	{
		this->write_cache( );
		write_fn(c, write__);
	}
}

void console::write(std::wstring&& str)
{
	_Write_line_helper(std::move(str), this, this);
}

void console::write(const std::wstring_view& str)
{
	_Write_line_helper((str), this, this);
}

void console::write_line(const std::wstring_view& str)
{
	runtime_assert(!str.empty( ), "String is empty!");

	auto stream = _Get_time_str<wchar_t>( ) << " - " << str << '\n';
	_Write_line_helper<false>(std::move(stream), this, this);
}
