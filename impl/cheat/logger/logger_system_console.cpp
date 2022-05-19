
#if 0

#include <cheat/tools/interface.h>

#include <nstd/runtime_assert.h>

#include <windows.h>

#include <cassert>
#include <chrono>
#include <corecrt_io.h>
#include <fcntl.h>
#include <fstream>
#include <intrin.h>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <variant>

import cheat.logger;
import nstd.mem.address;
import nstd.winapi.modules;

using namespace cheat;

template <typename... Chr>
using packed_string_t = std::variant<std::basic_string<Chr>..., std::basic_string_view<Chr>..., std::basic_ostringstream<Chr>..., Chr..., const Chr*...>;
using packed_string = packed_string_t<char, wchar_t>;

template <typename Chr, typename Tr>
static FILE* _Get_file_buff(std::basic_ios<Chr, Tr>& stream) noexcept
{
    using fb = std::basic_filebuf<Chr, Tr>;

    auto buff = stream.rdbuf();
    auto real_buff = dynamic_cast<fb*>(buff);
    assert(real_buff != nullptr);
    constexpr auto offset = sizeof(fb) - sizeof(void*) * 3;
    //_Myfile
    return nstd::mem::basic_address(real_buff).plus(offset).deref<1>();
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
concept stream_possible = requires(S& stream, T val)
{
    stream << val;
};

template <typename T>
concept have_char_type = requires
{
    typename T::char_type;
};

template <typename T>
concept have_value_type = requires
{
    typename T::value_type;
};

template <typename T>
concept have_view_function = requires(T obj)
{
    obj.view();
};

template <typename T>
concept have_array_access = requires(T obj)
{
    obj[0];
};

template <typename T>
constexpr auto _Get_char_type(const T& sample = {}) noexcept
{
    if constexpr (have_char_type<T>)
        return T::char_type();
    else if constexpr (have_value_type<T>)
        return T::value_type();
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
#pragma warning(disable : 4702 /*4459*/)

template <typename T, typename... Next>
static auto _Write_text_ex(const T& text, const Next&... other) noexcept
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
            static_assert(_Always_false<char_t>, __FUNCSIG__ ": Unsupported char type");
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
            _Set_mode(/*stdout*/ file_out, prev_mode);
    }
    else if constexpr (have_view_function<T>)
    {
        _Write_text_ex(text.view(), other...);
        return;
    }
    else
    {
        static_assert(_Always_false<T>, __FUNCSIG__ ": Unsupported string type");
    }

    if constexpr (sizeof...(Next) > 0)
        _Write_text_ex(other...);
};

#pragma warning(pop)

static auto _Write_text = []<typename... T>(const T&... text) { _Write_text_ex(text...); };

static auto _Prepare_assert_message(const char* expression, const char* message, const std::source_location& location) noexcept
{
    std::ostringstream msg;

    const auto append = [&]<typename Name, typename Value>(const Name name, const Value value, bool newline = true) {
        msg << name << ": " << value;
        if (newline)
            msg << '\n';
    };

    msg << "Assertion failed!\n\n";
    append("File", location.file_name());
    append("Line", location.line());
    append("Column", location.column());
    append("Function", location.function_name(), false);
    if (expression)
        append("\n\nExpression", expression, false);
    if (message)
        msg << "\nMessage" << message;

    return msg;
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
    ~console_controller()
    {
        if (!running_)
            return;
        stop();
    }

    console_controller()
    {
        start();
    }

    bool running() const noexcept
    {
        return running_;
    }

    void start() noexcept
    {

    }

    void stop() noexcept
    {

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
        const auto fn = [time = _Current_time_string()]<typename T>(const T& obj) { _Write_text(time, " - ", obj, '\n'); };
        const auto lock = std::scoped_lock(mtx_);
        std::visit(fn, str);
    }
};

enum class state : uint8_t
{
    unset,
    on,
    off
};

static state console_state = state::unset;
static auto controller = instance_of<console_controller>;

template <typename T>
static void _Log_impl(const T str) noexcept
{
    switch (console_state)
    {
    case state::unset:
        runtime_assert("Unknown console state");
        break;
    case state::off:
        runtime_assert("Console are disabled");
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

bool console::active() noexcept
{
    return console_state == state::on;
}

void console::enable() noexcept
{
    switch (console_state)
    {
    case state::on:
        runtime_assert("Console already enabled!");
        break;
    case state::off:
        controller->start();
    case state::unset:
        console_state = state::on;
        break;
    }
}

void console::disable() noexcept
{
    switch (console_state)
    {
    case state::off:
        runtime_assert("Console already disabled!");
        break;
    case state::on:
        if (controller.initialized())
            controller->stop();
    case state::unset:
        console_state = state::off;
        break;
    }
}

void console::log(const std::string_view str) noexcept
{
    if (!active())
        return;
    _Log(str);
}

void console::log(const std::wstring_view str) noexcept
{
    if (!active())
        return;
    _Log(str);
}

#endif

module cheat.logger.system_console;

#include <cheat/tools/interface.h>

#include <nstd/runtime_assert.h>

#include <windows>

#include <cassert>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <string>
#include <time.h>

class stream_descriptor
{
    int descriptor_;

  public:
    ~stream_descriptor()
    {
    }

    stream_descriptor() = default;

    stream_descriptor(FILE* const stream) : descriptor_(_fileno(stream))
    {
        // If stdout or stderr is not associated with an output stream (for example, in a Windows application without a console window),
        // the file descriptor returned is -2. In previous versions, the file descriptor returned was -1
        assert(descriptor_ >= 0);
    }

    operator int() const noexcept
    {
        return descriptor_;
    }
};

class stream_mode_changer
{
    stream_descriptor descriptor_;
    int prev_mode_ = -1, curr_mode_ = -1;

    int set_mode_impl(const int mode) const noexcept
    {
        // If you write data to a file stream, explicitly flush the code by using fflush before you use _setmode to change the mode
        const auto prev_mode = _setmode(descriptor_, mode);
        assert(prev_mode != -1 && "Unable to change mode");
        return prev_mode;
    }

    stream_mode_changer& operator=(const stream_mode_changer&) = default;

  public:
    ~stream_mode_changer()
    {
        if (prev_mode_ != -1)
            set_mode_impl(prev_mode_);
    }

    stream_mode_changer() = default;
    stream_mode_changer(FILE* const stream) : descriptor_(stream)
    {
    }

    stream_mode_changer(const stream_mode_changer&) = delete;

    stream_mode_changer(stream_mode_changer&& other) noexcept
    {
        *this = other;
        other.prev_mode_ = -1;
    }

    stream_mode_changer& operator=(stream_mode_changer&& other) noexcept
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    void set(const int mode) noexcept
    {
        // also may be done by 'fwide'
        if (curr_mode_ == mode)
            return;
        const auto prev_mode = set_mode_impl(mode);
        curr_mode_ = mode;
        if (prev_mode_ == -1)
            prev_mode_ = prev_mode;
    }

    template <typename C>
    void set() noexcept;

    template <>
    void set<char>() noexcept
    {
        set(_O_TEXT);
    }

    template <>
    void set<wchar_t>() noexcept
    {
        set(_O_U16TEXT);
    }

    stream_mode_changer release() noexcept
    {
        stream_mode_changer ret;
        using std::swap;
        swap(ret, *this);
        return ret;
    }
};

class file_stream
{
    FILE* stream_;
    bool redirected_;

    file_stream& operator=(const file_stream&) = default;

  public:
    ~file_stream()
    {
        if (!redirected_ || !stream_)
            return;
        fclose(stream_);
    }

    file_stream() : stream_(nullptr)
    {
    }

    file_stream(FILE* const stream) : stream_(stream), redirected_(false)
    {
    }

    file_stream(FILE* const old_stream, _In_z_ char const* file_name, _In_z_ char const* mode) : redirected_(true)
    {
        redirected_ = true;
        [[maybe_unused]] const auto err = freopen_s(&stream_, file_name, mode, old_stream);
        assert(err == NULL);
    }

    file_stream(const file_stream&) = delete;

    file_stream(file_stream&& other) noexcept
    {
        *this = other;
        other.stream_ = nullptr;
    }
    file_stream& operator=(file_stream&& other)
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    operator FILE*() const noexcept
    {
        return stream_;
    }
};

class time_buff final : std::streambuf
{
    std::string buff_;

    int_type overflow(int_type c) override
    {
        if (c != EOF)
            buff_.push_back(c);

        return c;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        buff_.append(s, static_cast<std::string::size_type>(n));
        return n;
    }

  public:
    template <size_t S = std::char_traits<char>::length("XX:XX:XX:XXX")>
    constexpr size_t max_size() const noexcept
    {
        return S;
    }

    time_buff()
    {
        using namespace std::chrono;
        using clock = system_clock;

        const auto current_time_point = clock::now();
        const auto current_time = clock::to_time_t(current_time_point);
        tm current_localtime;

        localtime_s(&current_localtime, std::addressof(current_time));

        const auto current_time_since_epoch = current_time_point.time_since_epoch();
        const auto current_milliseconds = duration_cast<milliseconds>(current_time_since_epoch).count() % 1000;
        const auto current_microseconds = duration_cast<microseconds>(current_time_since_epoch).count() % 1000;

        buff_.reserve(this->max_size());
        *static_cast<std::streambuf*>(this) << std::setfill('0') << std::put_time(&current_localtime, "%T") << ' ' << std::setw(3) << current_milliseconds << '.' << std::setw(3) << current_microseconds;
    }

    operator std::string_view() const noexcept
    {
        return buff_;
    }

    operator std::string() && noexcept
    {
        return buff_;
    }
};

// gap. unused
class reader
{
    file_stream stream_;

  public:
    reader(file_stream&& stream) : stream_(std::move(stream))
    {
    }
}

class writer
{
    file_stream stream_;
    stream_mode_changer changer_;
    std::mutex mtx_;

    template <typename C, typename Ch = std::char_traits<C>>
    void write_nolock(const std::basic_string_view<C, Ch> text) noexcept
    {
        changer_.set<C>();
        [[maybe_unused]] const auto written = _fwrite_nolock(text.data(), sizeof(C), text.size(), stream_);
        assert(written == text.size());
    }

  public:
    writer(file_stream&& stream) : stream_(std::move(stream)), changer_(stream_)
    {
    }

    template <typename C, typename Ch>
    void operator()(const std::basic_string_view<C, Ch> text) noexcept
    {
        time_buff buff;
        std::string time = std::move(buff);
        assert(time.size() == buff.max_size());

        constexpr bool can_reuse_buff = std::string().capacity() - buff.max_size() > 3;
        if constexpr (can_reuse_buff)
        {
            time += ' ';
            time += '-';
            time += ' ';
        }

        const std::scoped_lock lock(mtx_);
        changer_(_O_TEXT);
        write_nolock(time);
        if constexpr (!can_reuse_buff)
        {
            constexpr C dash[3] = {' ', '-', ' '};
            write_nolock<C>({dash, 3});
        }
        write_nolock(text);
    }
};

using cheat::logger;

class logger_system_console : public logger
{
    std::mutex mtx_;

    FILE* in_;
    writer out_;
    writer err_;

    HWND window_;
    bool running_ = false;

  protected:
    void log_impl(const std::string_view str) noexcept override
    {
        out_(str);
    }
    void log_impl(const std::wstring_view str) noexcept override
    {
        out_(str);
    }

  public:
    ~logger_system_console() override
    {
        logger_system_console::disable();
    }

    bool active() const noexcept override
    {
        return running_;
    }

    void enable() override
    {
        runtime_assert(!running_, "Already started");
        auto console_window = GetConsoleWindow();
        if (console_window)
        {
            window_ = nullptr;
            in_ = stdin;
            out_ = stdout;
            err_ = stderr;
        }
        else
        {
            const auto console_allocated = AllocConsole();
            runtime_assert(console_allocated, "Unable to allocate the console!");

            window_ = GetConsoleWindow();
            runtime_assert(window_ != nullptr, "Unable to get the console window");

            in_ = {"CONIN$", "r", stdin};
            out_ = {"CONOUT$", "w", stdout};
            err_ = {"CONOUT$", "w", stderr};

            // const auto window_title_set = SetConsoleTitleW(nstd::winapi::current_module()->FullDllName.Buffer);
            // runtime_assert(window_title_set, "Unable set console title");
        }

        // runtime_assert(IsWindowUnicode(console_window) == TRUE);
        // runtime_assert_add_handler(this);

        write_line("Started");
        running_ = true;
    }

    void disable() override
    {
        runtime_assert(running_, "Already stopped");
        // runtime_assert_remove_handler(this->id());

        if (window_)
        {
            FreeConsole();
            PostMessage(window_, WM_CLOSE, 0U, 0L);
        }
        else
        {
            write_line("Stopped");
        }

        running_ = false;
    }
};

CHEAT_OBJECT_IMPL(logger, logger_system_console);
