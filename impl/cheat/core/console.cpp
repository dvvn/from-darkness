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

	return std::basic_ostringstream<T>( )
		   << std::setfill(ZERO)
		   << std::put_time(&current_localtime, FMT_ARG)
		   << SPACE
		   << std::setw(3) << current_milliseconds
		   << DOT
		   << std::setw(3) << current_microseconds;
}

void console_data::write_cache( )
{
	for (auto& c: cache_)
	{
		std::invoke(*c);
	}

	cache_.clear( );
}

template <bool Assert = true>
static auto _Set_mode(FILE* file, int mode)
{
	const auto old_mode = _setmode(_fileno(file), mode);
	if constexpr (Assert)
		runtime_assert(old_mode != -1, "Unable to change mode");
	return old_mode;
}

console::~console( )
{
	if (data_.allocated)
	{
		FreeConsole( );
		PostMessage(data_.handle, WM_CLOSE, 0U, 0L);
	}
}

bool console::load_impl( )
{
	data_.handle = GetConsoleWindow( );
	if (data_.handle != nullptr)
	{
		data_.allocated = false;
	}
	else
	{
		//create new console window
		if (!AllocConsole( ))
		{
			runtime_assert("Unable to alloc console!");
			return false;
		}
		data_.allocated = true;

		data_.handle = GetConsoleWindow( );
		if (data_.handle == nullptr)
		{
			runtime_assert("Unable to get console window");
			return false;
		}

		const auto full_path = all_modules::get_ptr( )->current( ).full_path( );
		if (!SetConsoleTitle(full_path.data( )))
		{
			runtime_assert("Unable set console title");
			return false;
		}
	}

	runtime_assert(IsWindowUnicode(data_.handle) == true);
	return true;
}

template <typename Stream, typename T>
concept stream_possible = requires(Stream& stream, const T& obj)
{
	stream << obj;
};

template <typename Chr, typename Tr>
static auto _Get_file_buff(std::basic_ostream<Chr, Tr>& stream)
{
	using fb = std::basic_filebuf<Chr, Tr>;

	auto buff      = stream.rdbuf( );
	auto real_buff = dynamic_cast<fb*>(buff);
	runtime_assert(real_buff != nullptr);
	constexpr auto offset = sizeof(fb) - sizeof(void*) * 3;
	//_Myfile
	return address(real_buff).add(offset).ref<FILE*>( );
}

static auto _Write_text = []<typename T>(T&& text)
{
	FILE* file_out;
	int   new_mode;
	int   prev_mode;

	using value_type = std::remove_cvref_t<T>;
	if constexpr (stream_possible<std::ofstream, value_type>)
	{
		file_out  = _Get_file_buff(std::cout);
		new_mode  = _O_TEXT;
		prev_mode = _Set_mode(file_out, new_mode);
		std::cout << text;
	}
	else if constexpr (stream_possible<std::wofstream, value_type>)
	{
		file_out  = _Get_file_buff(std::wcout);
		new_mode  = _O_U16TEXT;
		prev_mode = _Set_mode(file_out, new_mode);
		std::wcout << text;
	}
	else
	{
		static_assert(std::_Always_false<value_type>, __FUNCSIG__);
		return;
	}

	if (prev_mode != new_mode)
		_Set_mode(/*stdout*/file_out, prev_mode);
};

static auto _Write_or_cache = []<typename T>(T&& text, const console* instance, console_data& data)
{
	const auto lock = std::scoped_lock(data.lock);

	auto fn = std::bind_front(_Write_text, std::forward<T>(text));
	if (!instance->state( ).done( ))
	{
		data.add_to_cache(std::move(fn));
	}
	else
	{
		data.write_cache( );
		std::invoke(fn/*, data.write*/);
	}
};

template <typename Chr>
static auto _Write_or_cache_full = []<typename T>(T&& text, const console* instance, console_data& data)
{
	constexpr Chr pad[] = {' ', '-', ' ', '\0'};

	auto stream = _Get_time_str<Chr>( ) << pad << std::forward<T>(text) << static_cast<Chr>('\n');
	_Write_or_cache(std::move(stream), instance, data);
};

void console::write(std::string&& str)
{
	_Write_or_cache(std::move(str), this, data_);
}

void console::write(const std::string_view& str)
{
	_Write_or_cache(str, this, data_);
}

void console::write_line(const std::string_view& str)
{
	_Write_or_cache_full<char>(str, this, data_);
}

void console::write(char c)
{
	_Write_or_cache(c, this, data_);
}

void console::write(std::wstring&& str)
{
	_Write_or_cache(std::move(str), this, data_);
}

void console::write(const std::wstring_view& str)
{
	_Write_or_cache(str, this, data_);
}

void console::write_line(const std::wstring_view& str)
{
	_Write_or_cache_full<wchar_t>(str, this, data_);
}

void console::write(std::ostringstream&& str)
{
	_Write_or_cache(std::move(str), this, data_);
}

void console::write(const std::ostringstream& str)
{
	_Write_or_cache(str.view( ), this, data_);
}

void console::write_line(const std::ostringstream& str)
{
	_Write_or_cache_full<char>(str.view( ), this, data_);
}

void console::write(std::wostringstream&& str)
{
	_Write_or_cache(std::move(str), this, data_);
}

void console::write(const std::wostringstream& str)
{
	_Write_or_cache(str.view( ), this, data_);
}

void console::write_line(const std::wostringstream& str)
{
	_Write_or_cache_full<wchar_t>(str.view( ), this, data_);
}
