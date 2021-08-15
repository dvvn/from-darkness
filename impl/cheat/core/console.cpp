#include "console.h"

using namespace cheat;
using namespace detail;
using namespace utl;

std::string timestamp( )
{
	using namespace std::chrono;
	using clock = system_clock;

	const auto current_time_point = clock::now( );
	const auto current_time       = clock::to_time_t(current_time_point);
	auto       current_localtime  = tm( );
	localtime_s(&current_localtime, &current_time);
	const auto current_time_since_epoch = current_time_point.time_since_epoch( );
	const auto current_milliseconds     = duration_cast<milliseconds>(current_time_since_epoch).count( ) % 1000;

	std::ostringstream stream;
	stream << std::put_time(&current_localtime, "%T") << "." << std::setw(3) << std::setfill('0') << current_milliseconds;
	return stream.str( );
}

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

	auto stream = std::ostringstream( );
	stream << std::setfill('0')
			<< std::put_time(&current_localtime, "%T")
			<< ' '
			<< std::setw(3) << current_milliseconds
			<< '.'
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

bool console::Do_load( )
{
#ifndef CHEAT_HAVE_CONSOLE
	return false;
#else
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
			runtime_assert("Unable to alloc console!");
		else
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
			runtime_assert("Unable to open stream");
		else
		{
			write__            = write;
			write_redirected__ = true;
		}

		const auto full_path = /*basic_string*/all_modules::get_ptr( )->current( ).full_path( );
		//ranges::replace(full_path, '\\', '/');
		(void)full_path;

		if (!SetConsoleTitle(full_path.data( )))
			runtime_assert("Unable set console title");

		console_window__ = GetConsoleWindow( );
		runtime_assert(console_window__ != nullptr, "Unable to get console window");
	}

	return true;
#endif
}

static auto _Write_line = []<typename T>(T&& text, FILE* file)
{
	if constexpr (!std::is_class_v<std::remove_cvref_t<T>>)
	{
		_Write_line(std::basic_string_view(text), file);
	}
	else
	{
		const auto size = text.size( );
		[[maybe_unused]]
				const auto written = std::fwrite(text.data( ), sizeof(std::remove_cvref_t<decltype(text[0])>), size, file);
		runtime_assert(written == size);
	}
};

template <bool CheckSize = true, typename T>
static void _Write_helper(T&& text, console* instance, console_data* data)
{
	if constexpr (CheckSize)
	{
		if constexpr (std::is_class_v<std::remove_cvref_t<T>>)
			runtime_assert(!text.empty( ), "String is empty!");
		else
			runtime_assert(!(text == nullptr || text[0] == '\0' || text[1] == '\0'), "String is empty!");

		if (std::size(text) == 1)
			return instance->write(text[0]);
	}

	const auto lock = std::scoped_lock(data->lock__);
	if (!instance->state( ).done( ))
	{
		data->cache__.push(std::bind_front(_Write_line, std::string(std::forward<T>(text))));
	}
	else
	{
		data->write_cache( );
		_Write_line(text, data->write__);
	}
}

void console::write(std::string&& str)
{
	_Write_helper(std::move(str), this, this);
}

void console::write(const std::string_view& str)
{
	_Write_helper(str, this, this);
}

void console::write_line(const std::string_view& str)
{
	runtime_assert(!str.empty( ), "String is empty!");

	const auto stream = _Get_time_str( ) << " - " << str << '\n';
	_Write_helper<false>(stream.str( ), this, this);
}

void console::write(char c)
{
	const auto lock = std::scoped_lock(lock__);

	constexpr auto write_fn = [](const char chr, FILE* file)
	{
		[[maybe_unused]] const auto written = std::fputc(chr, file);
		runtime_assert(written == chr);
	};

	if (!this->state( ).done( ))
		cache__.push(std::bind_front(write_fn, c));
	else
	{
		write_cache( );
		write_fn(c, write__);
	}
}
