#include "console.h"

#include <nstd/os/module info.h>

#include <Windows.h>
#include <corecrt_io.h>
#include <fcntl.h>

#include <fstream>
#include <functional>
#include <intrin.h>
#include <iostream>
#include <mutex>

using namespace cheat;

struct detail::string_packer::data_type: std::variant<str, strv, wstr, wstrv, ostr, wostr>
{
};

detail::string_packer::string_packer(const char* cstr)
{
	init( );
	packed->emplace<strv>(cstr);
}

detail::string_packer::string_packer(const wchar_t* wcstr)
{
	init( );
	packed->emplace<wstrv>(wcstr);
}

detail::string_packer::string_packer(str&& s)
{
	init( );
	packed->emplace<str>(std::move(s));
}

detail::string_packer::string_packer(const strv& s)
{
	init( );
	packed->emplace<strv>(s);
}

detail::string_packer::string_packer(wstr&& s)
{
	init( );
	packed->emplace<wstr>(std::move(s));
}

detail::string_packer::string_packer(const wstrv& s)
{
	init( );
	packed->emplace<wstrv>(s);
}

detail::string_packer::string_packer(ostr&& s)
{
	init( );
	packed->emplace<ostr>(std::move(s));
}

detail::string_packer::string_packer(const ostr& os)
{
	init( );
	packed->emplace<strv>(os.view( ));
}

detail::string_packer::string_packer(wostr&& os)
{
	init( );
	packed->emplace<wostr>(std::move(os));
}

detail::string_packer::string_packer(const wostr& os)
{
	init( );
	packed->emplace<wstrv>(os.view( ));
}

detail::string_packer::~string_packer( ) = default;

void detail::string_packer::init( )
{
	packed = std::make_unique<data_type>( );
}

struct movable_function_base
{
	virtual      ~movable_function_base( ) = default;
	virtual void operator()( ) = 0;

	movable_function_base( ) = default;

	movable_function_base(const movable_function_base&)            = delete;
	movable_function_base& operator=(const movable_function_base&) = delete;
};

template <typename T>
class movable_function final: public movable_function_base
{
public:
	movable_function(const movable_function&)            = delete;
	movable_function& operator=(const movable_function&) = delete;

	using value_type = T;

	movable_function(T&& obj)
		: obj_(std::move(obj))
	{
	}

	void operator()( ) override
	{
		std::invoke(obj_);
	}

private:
	T obj_;
};

template <typename T>
movable_function(T&&) -> movable_function<std::remove_cvref_t<T>>;

class console::cache_type
{
public:
	using value_type = std::unique_ptr<movable_function_base>;

	cache_type( ) = default;

	void write_all( )
	{
		std::ranges::for_each(cache_, &value_type::element_type::operator(), &value_type::operator*);
		cache_.clear( );
	}

	void store(value_type&& obj)
	{
		cache_.emplace_back(std::move(obj));
	}

	void lock( )
	{
		lock_.lock( );
	}

	void unlock( )
	{
		lock_.unlock( );
	}

private:
	std::recursive_mutex    lock_;
	std::vector<value_type> cache_;
};

console::console( )
{
	cache_ = std::make_unique<cache_type>( );
	nstd::rt_assert_object.add(this);
}

console::~console( )
{
	nstd::rt_assert_object.remove(this);

	if (this->allocated_)
	{
		FreeConsole( );
		PostMessage(this->handle_, WM_CLOSE, 0U, 0L);
	}

	if (in_)
		_fclose_nolock(in_);
	if (out_)
		_fclose_nolock(out_);
	if (err_)
		_fclose_nolock(err_);
}

service_base::load_result console::load_impl( )
{
	handle_ = GetConsoleWindow( );
	if (handle_ != nullptr)
	{
		allocated_ = false;
		//pad
	}
	else
	{
		//create new console window
		if (!AllocConsole( ))
		{
			runtime_assert("Unable to alloc console!");
			co_return service_state::error;
		}
		allocated_ = true;

		handle_ = GetConsoleWindow( );
		if (handle_ == nullptr)
		{
			runtime_assert("Unable to get console window");
			co_return service_state::error;
		}

		// ReSharper disable CppInconsistentNaming
		// ReSharper disable CppEnforceCVQualifiersPlacement
		constexpr auto _Freopen = [](_Outptr_result_maybenull_ FILE** _Stream, _In_z_ char const* _FileName, _In_z_ char const* _Mode, _Inout_ FILE* _OldStream)
			// ReSharper restore CppEnforceCVQualifiersPlacement
			// ReSharper restore CppInconsistentNaming
		{
			[[maybe_unused]] const auto err = freopen_s(_Stream, _FileName, _Mode, _OldStream);
			runtime_assert(err == NULL);
		};

		_Freopen(&in_, "CONIN$", "r", stdin);
		_Freopen(&out_, "CONOUT$", "w", stdout);
		_Freopen(&err_, "CONOUT$", "w", stderr);

		const auto full_path = nstd::os::all_modules::get_ptr( )->current( ).full_path( );
		if (!SetConsoleTitle(full_path.data( )))
		{
			runtime_assert("Unable set console title");
			co_return service_state::error;
		}
	}

	runtime_assert(IsWindowUnicode(handle_) == TRUE);
	co_return service_state::loaded;
}

void console::handle_impl(const nstd::rt_assert_arg_t& expression, const nstd::rt_assert_arg_t& message, const info_type& info) noexcept
{
	//static auto lock = std::mutex( );
	//const auto  _    = std::scoped_lock(lock);

#ifdef _DEBUG
	[[maybe_unused]] const auto from  = _ReturnAddress( );
	[[maybe_unused]] const auto from2 = _AddressOfReturnAddress( );
	DebugBreak( );
#endif

	auto msg = std::wostringstream( );
	msg << "Assertion falied!\n\n";

	const auto append = [&]<typename Name, typename Value>(Name&& name, Value&& value, bool last = false)
	{
		msg << name << ": " << value;
		if (!last)
			msg << '\n';
	};

	if (!expression.empty( ))
		append("Expression", expression);

	append("File", info.file_name);
	append("Line", info.line);
	append("Function", info.function, message.empty( ));

	if (!message.empty( ))
		msg << '\n' << message;

	this->write_line(std::move(msg));

	[[maybe_unused]] static volatile auto skip_helper = '\1';
}

template <typename Chr, typename Tr>
static FILE* _Get_file_buff(std::basic_ios<Chr, Tr>& stream)
{
	using fb = std::basic_filebuf<Chr, Tr>;

	auto buff      = stream.rdbuf( );
	auto real_buff = dynamic_cast<fb*>(buff);
	runtime_assert(real_buff != nullptr);
	constexpr auto offset = sizeof(fb) - sizeof(void*) * 3;
	//_Myfile
	return nstd::address(real_buff).add(offset).ref<FILE*>( );
}

template <bool Assert = true>
static auto _Set_mode(FILE* file, int mode)
{
	const auto old_mode = _setmode(_fileno(file), mode);
	if constexpr (Assert)
		runtime_assert(old_mode != -1, "Unable to change mode");
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
static decltype(auto) _Unwrap_view(T&& text)
{
	if constexpr (is_basic_ostringstream_v<std::remove_cvref_t<T>>)
		return text.view( );
	else
		return std::forward<T>(text);
}

using console_cache_type_uptr = std::unique_ptr<console::cache_type>;
static auto _Write_text = []<typename T>(T&& text)
{
	FILE* file_out;
	int   new_mode;
	int   prev_mode;

	auto&& text_fwd = _Unwrap_view(std::forward<T>(text));
	using value_type = std::remove_cvref_t<decltype(text_fwd)>;
	if constexpr (stream_possible<std::ofstream&, value_type>)
	{
		file_out  = _Get_file_buff(std::cin);
		new_mode  = _O_TEXT;
		prev_mode = _Set_mode(file_out, new_mode);
		std::cout << text_fwd;
	}
	else if constexpr (stream_possible<std::wofstream&, value_type>)
	{
		file_out  = _Get_file_buff(std::wcin);
		new_mode  = _O_U16TEXT;
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

template <typename Fn, typename T>
static auto _Decayed_bind(const Fn& fn, T&& text)
{
	auto get_text = [&]( )-> decltype(auto)
	{
		if constexpr (std::is_rvalue_reference_v<decltype(text)>)
			return std::forward<T>(text);
		else
		{
			auto&& str = _Unwrap_view(std::forward<T>(text));
			using str_t = decltype(str);
			using str_raw_t = std::remove_cvref_t<str_t>;

			if constexpr (!std::_Has_member_value_type<str_raw_t>)
				return std::forward<str_t>(str);
			else
			{
				using chr_t = typename str_raw_t::value_type;
				return std::basic_string<chr_t>(std::forward<str_t>(str));
			}
		}
	};

	return std::bind_front(fn, get_text( ));
}

static auto _Write_or_cache = []<typename T>(T&& text, const console* instance, console_cache_type_uptr& cache)
{
	const auto lock = std::scoped_lock(*cache);

	if (instance->state( ) == service_state::loaded)
	{
		cache->write_all( );
		_Write_text(std::forward<T>(text));
	}
	else
	{
		auto fn = _Decayed_bind(_Write_text, std::forward<T>(text));
		using fn_t = decltype(fn);
		auto vfn = std::make_unique<movable_function<fn_t>>(std::move(fn));
		cache->store(std::move(vfn));
	}
};

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

template <class T>
// ReSharper disable once CppInconsistentNaming
concept _Has_member_char_type = requires
{
	typename T::char_type;
};

template <typename T>
static auto _Detect_char_type( )
{
	if constexpr (_Has_member_char_type<T>)
		return T::char_type( );
	else if constexpr (std::_Has_member_value_type<T>)
		return T::value_type( );
	else
		return char( );
}

static auto _Write_or_cache_full = []<typename T>(T&& text, const console* instance, console_cache_type_uptr& cache)
{
	using Chr = decltype(_Detect_char_type<std::remove_cvref_t<T>>( ));

	constexpr Chr pad[] = {' ', '-', ' ', '\0'};

	auto stream = _Get_time_str<Chr>( ) << pad << _Unwrap_view(text) << static_cast<Chr>('\n');
	_Write_or_cache(std::move(stream), instance, cache);
};

void console::write(char c)
{
	_Write_or_cache(c, this, cache_);
}

template <typename Fn, typename ...Args>
static void _Pack(detail::string_packer& str, Fn&& fn, Args&&...args)
{
	auto packed = [&]<typename T>(T&& s)
	{
		fn(std::forward<T>(s), args...);
	};

	std::visit(packed, *str.packed);
}

void console::write(detail::string_packer&& str)
{
	_Pack(str, _Write_or_cache, this, cache_);
}

void console::write_line(detail::string_packer&& str)
{
	_Pack(str, _Write_or_cache_full, this, cache_);
}
