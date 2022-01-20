module;

#include "includes.h"

#include <nstd/rtlib/includes.h>
#include <nstd/type_traits.h>

#include <corecrt_io.h>
#include <fcntl.h>
#include <intrin.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iomanip>

module cheat.console;
import cheat.csgo.awaiter;
import nstd.rtlib;

using namespace cheat;

static auto _Prepare_message(const char* expression, const char* message, const std::source_location& location)
{
	std::ostringstream msg;

	const auto append = [&]<typename Name, typename Value>(Name && name, Value && value, bool newline = true)
	{
		msg << name << ": " << value;
		if (newline)
			msg << '\n';
	};

	msg << "Assertion failed!\n\n";
	append("File", location.file_name( ));
	append("Line", location.line( ));
	append("Column", location.column( ));
	append("Function", location.function_name( ), false);
	if (expression)
		append("\n\nExpression", expression, false);
	if (message)
		msg << "\nMessage" << message;

	return msg;
}

void console::handle(bool expression_result, const char* expression, const char* message, const std::source_location& location) noexcept
{
	if (expression_result)
		return;

	write_line(_Prepare_message(expression, message, location));
}

void console::handle(const char* message, const std::source_location& location) noexcept
{
	write_line(_Prepare_message(nullptr, message, location));
}

size_t console::id( ) const
{
	return reinterpret_cast<size_t>(this);
}

console::console( ) = default;

console::~console( )
{
	runtime_assert_remove_handler(this->id( ));

	if (this->allocated_)
	{
		FreeConsole( );
		PostMessage(this->handle_, WM_CLOSE, 0U, 0L);
	}

	if (in_)
		fclose(in_);
	if (out_)
		fclose(out_);
	if (err_)
		fclose(err_);
}

void console::construct( )noexcept
{
	this->deps( ).add<csgo_awaiter>( );
}

bool console::load( ) noexcept
{
	handle_ = GetConsoleWindow( );
	if (handle_ != nullptr)
	{
		allocated_ = false;
	}
	else
	{
		//create new console window
		if (!AllocConsole( ))
		{
			runtime_assert("Unable to alloc console!");
			return false;
		}

		allocated_ = true;

		handle_ = GetConsoleWindow( );
		if (handle_ == nullptr)
		{
			runtime_assert("Unable to get console window");
			return false;
		}

		// ReSharper disable CppInconsistentNaming
		// ReSharper disable CppEnforceCVQualifiersPlacement
		constexpr auto _Freopen = [](_Outptr_result_maybenull_ FILE*& _Stream, _In_z_ char const* _FileName, _In_z_ char const* _Mode, _Inout_ FILE* _OldStream)
		{
			[[maybe_unused]] const auto err = freopen_s(std::addressof(_Stream), _FileName, _Mode, _OldStream);
			runtime_assert(err == NULL);
		};
		// ReSharper restore CppEnforceCVQualifiersPlacement
		// ReSharper restore CppInconsistentNaming

		_Freopen(in_, "CONIN$", "r", stdin);
		_Freopen(out_, "CONOUT$", "w", stdout);
		_Freopen(err_, "CONOUT$", "w", stderr);

		const std::wstring_view full_path = nstd::rtlib::all_infos::get( ).current( ).full_path( ).raw;
		if (!SetConsoleTitleW(full_path.data( )))
		{
			runtime_assert("Unable set console title");
			return false;
		}
	}

	runtime_assert_add_handler(this);
	runtime_assert(IsWindowUnicode(handle_) == TRUE);
	return true;
}

template <typename Chr, typename Tr>
static FILE* _Get_file_buff(std::basic_ios<Chr, Tr>&stream)
{
	using fb = std::basic_filebuf<Chr, Tr>;

	auto buff = stream.rdbuf( );
	auto real_buff = dynamic_cast<fb*>(buff);
	assert(real_buff != nullptr);
	constexpr auto offset = sizeof(fb) - sizeof(void*) * 3;
	//_Myfile
	return nstd::mem::address(real_buff).add(offset).ref( );
}

template <bool Assert = true>
static auto _Set_mode(FILE * file, int mode)
{
	const auto old_mode = _setmode(_fileno(file), mode);
	if constexpr (Assert)
		assert(old_mode != -1 && "Unable to change mode");
	return old_mode;
}

template <typename S, typename T>
concept stream_possible = requires(S stream, T val)
{
	stream << val;
};

template <typename T>
constexpr auto is_basic_ostringstream_v = false;

template <typename E, typename Tr, typename A>
constexpr auto is_basic_ostringstream_v<std::basic_ostringstream<E, Tr, A>> = true;

template <typename T>
static decltype(auto) _Unwrap_view(T && text)
{
	if constexpr (is_basic_ostringstream_v<std::remove_cvref_t<T>>)
		return text.view( );
	else
		return std::forward<T>(text);
}

static auto _Write_text = []<typename T>(T && text)
{
	FILE* file_out;
	int new_mode;
	int prev_mode;

	decltype(auto) text_fwd = _Unwrap_view(std::forward<T>(text));
	using value_type = std::remove_cvref_t<decltype(text_fwd)>;
	if constexpr (stream_possible<std::ofstream&, value_type>)
	{
		file_out = _Get_file_buff(std::cin);
		new_mode = _O_TEXT;
		prev_mode = _Set_mode(file_out, new_mode);
		std::cout << text_fwd;
	}
	else if constexpr (stream_possible<std::wofstream&, value_type>)
	{
		file_out = _Get_file_buff(std::wcin);
		new_mode = _O_U16TEXT;
		prev_mode = _Set_mode(file_out, new_mode);
		std::wcout << text_fwd;
	}
	else
	{
		static_assert(std::_Always_false<value_type>, __FUNCSIG__);
		return;
	}

	if (prev_mode != new_mode)
		_Set_mode(/*stdout*/file_out, prev_mode);
};

template <std::_Has_member_value_type T>
static auto _Make_string(const T & str)
{
	auto bg = str.begin( );
	auto ed = str.end( );

	if constexpr (nstd::_Has_member_allocator_type<T>)
		return std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>(bg, ed);
	else
		return std::basic_string<typename T::value_type, typename T::traits_type>(bg, ed);
}

template <class T>
	requires(!std::is_class_v<T>)
static auto _Make_string(const T * ptr) { return std::basic_string<T>(ptr); }

template <class T>
	requires(!std::is_class_v<T>)
static auto _Make_string(const T val) { return val; }

template <class T>
	requires(is_basic_ostringstream_v<T>)
static auto _Make_string(const T & ostr) { return ostr.str( ); }

template <typename Fn, typename T>
static auto _Decayed_bind(Fn && fn, T && text)
{
	const auto get_text = [&]( )-> decltype(auto)
	{
		if constexpr (std::is_rvalue_reference_v<decltype(text)>)
			return std::forward<T>(text);
		else
			return _Make_string(text);
	};

	return std::bind_front(std::forward<Fn>(fn), get_text( ));
}

static auto _Write_or_cache = []<typename T>(T && text, const console * instance, console_cache & cache)
{
	const auto lock = std::scoped_lock(cache);

	if (instance->state( ) == service_state::loaded)
	{
		cache.write_all( );
		_Write_text(std::forward<T>(text));
	}
	else
	{
		auto fn = _Decayed_bind(_Write_text, std::forward<T>(text));
		if constexpr (std::copyable<decltype(fn)>)
			cache.store(fn);
		else
			cache.store([fn1 = std::make_shared<decltype(fn)>(std::move(fn))]{std::invoke(*fn1);});
	}
};

template <typename T>
static auto _Get_time_str( )
{
	using namespace std::chrono;
	using clock = system_clock;

	const auto current_time_point = clock::now( );
	const auto current_time = clock::to_time_t(current_time_point);
	auto current_localtime = tm( );

	localtime_s(&current_localtime, &current_time);

	const auto current_time_since_epoch = current_time_point.time_since_epoch( );
	const auto current_milliseconds = duration_cast<milliseconds>(current_time_since_epoch).count( ) % 1000;
	const auto current_microseconds = duration_cast<microseconds>(current_time_since_epoch).count( ) % 1000;

	// ReSharper disable CppInconsistentNaming
	constexpr T ZERO = '0';
	constexpr T SPACE = ' ';
	constexpr T DOT = '.';

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

static auto _Write_or_cache_full = []<typename T>(T && text, const console * instance, console_cache & cache)
{
	decltype(auto) view = _Unwrap_view(text);

	using Chr = std::iter_value_t<decltype(view)>;
	constexpr Chr pad[] = {' ', '-', ' ', '\0'};

	auto stream = _Get_time_str<Chr>( ) << pad << view << static_cast<Chr>('\n');
	_Write_or_cache(std::move(stream), instance, cache);
};

void console::write(char c)
{
	_Write_or_cache(c, this, cache_);
}

template <typename Fn, typename ...Args>
static void _Pack(string_packer& str, Fn&& fn, Args&&...args)
{
	auto packed = [&]<typename T>(T && s)
	{
		fn(std::forward<T>(s), args...);
	};

	std::visit(packed, str.packed);
}

void console::write(string_packer&& str)
{
	_Pack(str, _Write_or_cache, this, cache_);
}

void console::write_line(string_packer&& str)
{
	_Pack(str, _Write_or_cache_full, this, cache_);
}