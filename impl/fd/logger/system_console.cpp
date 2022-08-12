module;

#include <fd/assert.h>
#include <fd/object.h>

#include <Windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <time.h>

module fd.system_console;
#ifdef _DEBUG
import fd.to_char;
#endif
import fd.chars_cache;
import fd.format;
import fd.mutex;

using namespace fd;

class file_stream
{
    FILE* stream_;
    bool redirected_;

    file_stream(const file_stream&)            = default;
    file_stream& operator=(const file_stream&) = default;

  public:
    ~file_stream()
    {
        if (!redirected_ || !stream_)
            return;
        _fclose_nolock(stream_);
    }

    file_stream()
        : stream_(nullptr)
    {
    }

    file_stream(FILE* const stream)
        : stream_(stream)
        , redirected_(false)
    {
    }

    file_stream(_In_z_ char const* file_name, _In_z_ char const* mode, FILE* const old_stream)
        : redirected_(true)
    {
        redirected_                     = true;
        [[maybe_unused]] const auto err = freopen_s(&stream_, file_name, mode, old_stream);
        FD_ASSERT(err == NULL);
    }

    file_stream(file_stream&& other)
    {
        *this         = other;
        other.stream_ = nullptr;
    }

    file_stream& operator=(file_stream&& other)
    {
        const auto old  = *this;
        *this           = other;
        other           = old;
        return *this;
    }

    operator FILE*() const
    {
        return stream_;
    }
};

static string _Get_current_time()
{
    using namespace std::chrono;
    using clock = system_clock;

#if 1
    return format("{:%T}", current_zone()->to_local(clock::now()).time_since_epoch());
#else
    const auto current_time_point = clock::now();
    const auto current_time       = clock::to_time_t(current_time_point);
    tm current_localtime;

    localtime_s(&current_localtime, std::addressof(current_time));

    const auto current_time_since_epoch = current_time_point.time_since_epoch();
    const auto current_milliseconds     = duration_cast<milliseconds>(current_time_since_epoch).count() % 1000;
    const auto current_microseconds     = duration_cast<microseconds>(current_time_since_epoch).count() % 1000;

    std::ostringstream ss;
    ss << std::setfill('0') << std::put_time(&current_localtime, "%T") << ' ' << std::setw(3) << current_milliseconds << '.' << std::setw(3) << current_microseconds;
    return std::move(ss).str();
#endif
}

#define putc_assert(_RESULT_) FD_ASSERT(_RESULT_ != WEOF, errno == EILSEQ ? "Encoding error in putc." : "I/O error in putc.")

// gap. unused
class reader
{
    file_stream stream_;

  public:
    reader() = default;

    reader(file_stream&& stream)
        : stream_(std::move(stream))
    {
    }
};

class writer
{
    file_stream stream_;
    mutex mtx_;

    void write(const wchar_t* ptr, const size_t size)
    {
        // [[maybe_unused]] const auto ok = std::fputws(ptr, stream_);
        // putc_assert(ok);

        FILE* const f = stream_;
#ifdef _WIN32
        const auto fd = _fileno(f);
        if (_isatty(fd))
        {
            DWORD written;
            if (WriteConsoleW(reinterpret_cast<void*>(_get_osfhandle(fd)), ptr, static_cast<DWORD>(size), &written, nullptr))
                return;

            // Fallback to fwrite on failure. It can happen if the output has been
            // redirected to NUL.
        }
#endif
        _fwrite_nolock(ptr, sizeof(wchar_t), size, f);
    }

    void write(const char* ptr, const size_t size)
    {
        /* const auto wstr = to_char<wchar_t>(string_view(ptr, size));
         write(wstr.data(), wstr.size()); */
        _fwrite_nolock(ptr, sizeof(char), size, stream_);
    }

  public:
    writer() = default;

    writer(file_stream&& stream)
        : stream_(std::move(stream))
    {
    }

    writer& operator=(file_stream&& stream)
    {
        stream_ = std::move(stream);
        return *this;
    }

    template <typename C, class Locker>
    void operator()(const basic_string_view<C> text, const Locker locker)
    {
        const auto time = _Get_current_time();

        const auto lock = locker(mtx_);
        write(time.data(), time.size());
        write(" - ", 3);
        write(text.data(), text.size());
        write("\n", 1);
    }

    template <typename C, class Locker>
    void operator()(const C* text, const Locker locker)
    {
        invoke(*this, basic_string_view(text), locker);
    }
};

constexpr auto real_locker = [](auto& mtx) {
    return lock_guard(mtx);
};

constexpr auto fake_locker = [](auto&) {
    return nullptr;
};

class console_writer_impl : public console_writer
{
    reader in_;
    writer out_;
    writer err_;

    HWND window_ = nullptr;

  public:
    void operator()(const string_view str) override
    {
        out_(str, real_locker);
    }

    void operator()(const wstring_view wstr) override
    {
        out_(wstr, real_locker);
    }

    ~console_writer_impl() override
    {
        if (window_)
        {
            FreeConsole();
            PostMessage(window_, WM_CLOSE, 0U, 0L);
        }
        else
        {
            out_("Stopped", fake_locker);
        }
    }

    console_writer_impl()
    {
        auto console_window = GetConsoleWindow();
        if (!console_window)
        {
            const auto console_allocated = AllocConsole();
            FD_ASSERT(console_allocated, "Unable to allocate the console!");

            console_window = GetConsoleWindow();
            FD_ASSERT(console_window, "Unable to get the console window");

            // const auto window_title_set = SetConsoleTitleW(nstd::winapi::current_module()->FullDllName.Buffer);
            // FD_ASSERT(window_title_set, "Unable set console title");

            window_ = console_window;
        }

        constexpr auto construct_helper = [](auto& obj, auto... args) {
            std::destroy_at(&obj);
            std::construct_at(&obj, file_stream(args...));
        };

        construct_helper(in_, "CONIN$", "r", stdin);
        construct_helper(out_, "CONOUT$", "w", stdout);
        construct_helper(err_, "CONOUT$", "w", stderr);

        FD_ASSERT(IsWindowUnicode(console_window) == TRUE);

        out_("Started", fake_locker);
    }
};

FD_OBJECT_BIND_TYPE(system_console_writer, console_writer_impl);
