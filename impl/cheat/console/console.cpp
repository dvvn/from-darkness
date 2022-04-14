module;

#include <nstd/runtime_assert.h>
#include <nstd/type_traits.h>

#include <windows.h>

#include <corecrt_io.h>
#include <fcntl.h>
#include <intrin.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <variant>
#include <chrono>
#include <mutex>

module cheat.console;
import nstd.mem.address;
import nstd.winapi.modules;
import nstd.one_instance;

using namespace cheat;

template<typename ...Chr>
using packed_string_t = std::variant<std::basic_string<Chr>..., std::basic_string_view<Chr>..., std::basic_ostringstream<Chr>..., Chr..., const Chr*...>;
using packed_string = packed_string_t<char, wchar_t>;

template <typename Chr, typename Tr>
static FILE* _Get_file_buff(std::basic_ios<Chr, Tr>& stream) noexcept
{
	using fb = std::basic_filebuf<Chr, Tr>;

	auto buff = stream.rdbuf( );
	auto real_buff = dynamic_cast<fb*>(buff);
	assert(real_buff != nullptr);
	constexpr auto offset = sizeof(fb) - sizeof(void*) * 3;
	//_Myfile
	return nstd::mem::basic_address(real_buff).plus(offset).deref<1>( );
}

template <bool Assert = true>
static auto _Set_mode(FILE* file, int mode) noexcept
{
	const auto old_mode = _setmode(_fileno(file), mode);
	if constexpr (Assert)
		assert(old_mode != -1 && "Unable to change mode");
	return old_mode;
}

template <bool Assert = true>
static auto _Set_mode(int known_prev_mode, FILE* file, int mode) noexcept
{
	if (known_prev_mode == mode)
		return mode;
	return _Set_mode<Assert>(file, mode);
}

template <typename S, typename T>
concept stream_possible = requires(S & stream, T val)
{
	stream << val;
};

template<typename T>
concept have_char_type = requires
{
	typename T::char_type;
};

template<typename T>
concept have_value_type = requires
{
	typename T::value_type;
};

template<typename T>
concept have_view_function = requires(T obj)
{
	obj.view( );
};

template<typename T>
concept have_array_access = requires(T obj)
{
	obj[0];
};

template<typename T>
constexpr auto _Get_char_type(const T& sample = {}) noexcept
{
	if constexpr (have_char_type<T>)
		return T::char_type( );
	else if constexpr (have_value_type<T>)
		return T::value_type( );
	else if constexpr (have_array_access<T>)
		return sample[0];
	else if constexpr (!std::is_class_v<T>)
		return sample;
}

#ifdef _MSC_VER
using std::_Always_false;
#else
TODO
#endif

#pragma warning(push)
#pragma warning(disable: 4702/*4459*/)

template< typename T, typename ...Next>
static auto _Write_text_ex(const T& text, const Next&...other) noexcept
{
	constexpr auto writable = stream_possible<std::ofstream, T>;
	constexpr auto writable_wide = stream_possible<std::wofstream, T>;
	constexpr auto universal = writable && writable_wide;

	if constexpr (universal)
	{
		using char_t = decltype(_Get_char_type(text));
		if constexpr (std::same_as<char_t, std::ofstream::char_type>)
			std::cout << text;
		else if constexpr (std::same_as<char_t, std::wofstream::char_type>)
			std::wcout << text;
		else
			static_assert(_Always_false<char_t>, __FUNCSIG__": Unsupported char type");
	}
	else if constexpr (writable || writable_wide)
	{
		FILE* file_out;
		int new_mode;
		int prev_mode;

		if constexpr (writable)
		{
			file_out = _Get_file_buff(std::cin);
			new_mode = _O_TEXT;
			prev_mode = _Set_mode(file_out, new_mode);
			std::cout << text;
		}
		else if constexpr (writable_wide)
		{
			file_out = _Get_file_buff(std::wcin);
			new_mode = _O_U16TEXT;
			prev_mode = _Set_mode(file_out, new_mode);
			std::wcout << text;
		}

		if (prev_mode != new_mode)
			_Set_mode(/*stdout*/file_out, prev_mode);
	}
	else if constexpr (have_view_function<T>)
	{
		_Write_text_ex(text.view( ), other...);
		return;
	}
	else
	{
		static_assert(_Always_false<T>, __FUNCSIG__": Unsupported string type");
	}

	if constexpr (sizeof...(Next) > 0)
		_Write_text_ex(other...);
};

#pragma warning(pop)

static auto _Write_text = []<typename ...T>(const T & ...text)
{
	_Write_text_ex(text...);
};

static auto _Prepare_assert_message(const char* expression, const char* message, const std::source_location& location) noexcept
{
	std::ostringstream msg;

	const auto append = [&]<typename Name, typename Value>(const Name name, const Value value, bool newline = true)
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

static auto _Current_time_string( ) noexcept
{
	using namespace std::chrono;
	using clock = system_clock;

	const auto current_time_point = clock::now( );
	const auto current_time = clock::to_time_t(current_time_point);
	tm current_localtime;

	localtime_s(&current_localtime, std::addressof(current_time));

	const auto current_time_since_epoch = current_time_point.time_since_epoch( );
	const auto current_milliseconds = duration_cast<milliseconds>(current_time_since_epoch).count( ) % 1000;
	const auto current_microseconds = duration_cast<microseconds>(current_time_since_epoch).count( ) % 1000;

	return std::ostringstream( ) << std::setfill('0')
		<< std::put_time(&current_localtime, "%T")
		<< ' '
		<< std::setw(3) << current_milliseconds
		<< '.'
		<< std::setw(3) << current_microseconds;
}

class console_controller : nstd::rt_assert_handler
{
	std::mutex mtx_;

	FILE* in_;
	FILE* out_;
	FILE* err_;

	HWND window_;
	bool running_ = false;

public:
	~console_controller( )
	{
		if (!running_)
			return;
		stop( );
	}

	console_controller( )
	{
		start( );
	}

	bool running( ) const noexcept
	{
		return running_;
	}

	void start( ) noexcept
	{
		runtime_assert(!running_, "Already started");
		auto console_window = GetConsoleWindow( );
		if (console_window)
		{
			window_ = nullptr;
			in_ = out_ = err_ = nullptr;
		}
		else
		{
			const auto window_created = AllocConsole( );
			runtime_assert(window_created, "Unable to allocate the console!");

			console_window = GetConsoleWindow( );
			runtime_assert(console_window != nullptr, "Unable to get console window");

			// ReSharper disable CppInconsistentNaming
			// ReSharper disable CppEnforceCVQualifiersPlacement
			constexpr auto _Freopen = [](_Outptr_result_maybenull_ FILE*& _Stream, _In_z_ char const* _FileName, _In_z_ char const* _Mode, _Inout_ FILE* _OldStream)
			{
				[[maybe_unused]]
				const auto err = freopen_s(std::addressof(_Stream), _FileName, _Mode, _OldStream);
				runtime_assert(err == NULL);
			};
			// ReSharper restore CppEnforceCVQualifiersPlacement
			// ReSharper restore CppInconsistentNaming

			_Freopen(in_, "CONIN$", "r", stdin);
			_Freopen(out_, "CONOUT$", "w", stdout);
			_Freopen(err_, "CONOUT$", "w", stderr);

			const auto window_title_set = SetConsoleTitle(nstd::winapi::current_module( )->FullDllName.Buffer);
			runtime_assert(window_title_set, "Unable set console title");

			window_ = console_window;
		}

		//runtime_assert(IsWindowUnicode(console_window) == TRUE);
		runtime_assert_add_handler(this);

		write_line("Started");
		running_ = true;
	}

	void stop( ) noexcept
	{
		runtime_assert(running_, "Already stopped");
		runtime_assert_remove_handler(this->id( ));

		if (window_)
		{
			FreeConsole( );
			PostMessage(window_, WM_CLOSE, 0U, 0L);
		}
		else
		{
			write_line("Stopped");
		}

		constexpr auto _Fclose = [](FILE*& f) noexcept
		{
			if (!f)
				return;
			fclose(f);
			f = nullptr;
		};


		_Fclose(in_);
		_Fclose(out_);
		_Fclose(err_);

		running_ = false;
	}

private:
	void handle(const char* expression, const char* message, const std::source_location& location) noexcept
	{
		write_line(_Prepare_assert_message(expression, message, location));
	}

	void handle(const char* message, const std::source_location& location) noexcept
	{
		write_line(_Prepare_assert_message(nullptr, message, location));
	}

public:
	void write(packed_string&& str) noexcept
	{
		const auto lock = std::scoped_lock(mtx_);
		std::visit(_Write_text, str);
	}

	void write_line(packed_string&& str) noexcept
	{
		const auto fn = [time = _Current_time_string( )]<typename T>(const T & obj)
		{
			_Write_text(time, " - ", obj, '\n');
		};
		const auto lock = std::scoped_lock(mtx_);
		std::visit(fn, str);
	}
};

enum class state :uint8_t
{
	unset, on, off
};

static state console_state = state::unset;
static nstd::one_instance_obj<console_controller> controller;

template<typename T>
static void _Log_impl(const T str) noexcept
{
	switch (console_state)
	{
	case state::unset:
		runtime_assert("Unknown console state");
		break;
	case state::on:
		controller->write_line(str);
		break;
	}
}

void _Log(const std::string_view str) noexcept
{
	_Log_impl(str);
}

void _Log(const std::wstring_view str) noexcept
{
	_Log_impl(str);
}

bool _Active( ) noexcept
{
	return console_state == state::on;
}

void console::enable( ) noexcept
{
	switch (console_state)
	{
	case state::on:
		runtime_assert("Console already enabled!");
		break;
	case state::off:
		controller->start( );
	case state::unset:
		console_state = state::on;
		break;
	}
}

void console::disable( ) noexcept
{
	switch (console_state)
	{
	case state::off:
		runtime_assert("Console already disabled!");
		break;
	case state::on:
		if (controller.initialized( ))
			controller->stop( );
	case state::unset:
		console_state = state::off;
		break;
	}
}

void console::log(const std::string_view str) noexcept
{
	if (!_Active( ))
		return;
	_Log(str);
}

void console::log(const std::wstring_view str) noexcept
{
	if (!_Active( ))
		return;
	_Log(str);
}