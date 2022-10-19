module;

#include <fd/assert.h>

#include <Windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <time.h>

module fd.system.console;
import fd.format;



using namespace fd;

file_stream::~file_stream()
{
    if (!redirected_ || !stream_)
        return;
    _fclose_nolock(stream_);
}

file_stream::file_stream()
    : stream_(nullptr)
{
}

file_stream::file_stream(FILE* stream)
    : stream_(stream)
    , redirected_(false)
{
}

file_stream::file_stream(const char* file_name, const char* mode, FILE* old_stream)
    : redirected_(true)
{
    [[maybe_unused]] const auto err = freopen_s(&stream_, file_name, mode, old_stream);
    FD_ASSERT(err == NULL);
}

file_stream::file_stream(file_stream&& other)
{
    *this         = other;
    other.stream_ = nullptr;
}

file_stream& file_stream::operator=(file_stream&& other)
{
    const auto old = *this;
    *this          = other;
    other          = old;
    return *this;
}

file_stream::operator void*() const
{
    return stream_;
}

file_stream::operator FILE*() const
{
    return stream_;
}

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

file_stream_reader::file_stream_reader() = default;

file_stream_reader& file_stream_reader::operator=(file_stream&& stream)
{
    stream_ = std::move(stream);
    return *this;
}

void fd::file_stream_writer::lock() noexcept
{
    mtx_.lock();
}

void fd::file_stream_writer::unlock() noexcept
{
    mtx_.unlock();
}

void file_stream_writer::write_nolock(const wchar_t* ptr, const size_t size)
{
    // [[maybe_unused]] const auto ok = std::fputws(ptr, stream_);
    // putc_assert(ok);

#ifdef _WIN32
    const auto h = _fileno(stream_);
    if (_isatty(h))
    {
        DWORD written;
        if (WriteConsoleW(reinterpret_cast<void*>(_get_osfhandle(h)), ptr, static_cast<DWORD>(size), &written, nullptr))
            return;

        // Fallback to fwrite on failure. It can happen if the output has been
        // redirected to NUL.
    }
#endif
    _fwrite_nolock(ptr, sizeof(wchar_t), size, stream_);
}

void file_stream_writer::write_nolock(const char* ptr, const size_t size)
{
    /* const auto wstr = utf_convert<wchar_t>(string_view(ptr, size));
     write(wstr.data(), wstr.size()); */
    _fwrite_nolock(ptr, sizeof(char), size, stream_);
}

void file_stream_writer::write(const wchar_t* ptr, const size_t size)
{
    const lock_guard guard = mtx_;
    write_nolock(ptr, size);
}

void file_stream_writer::write(const char* ptr, const size_t size)
{
    const lock_guard guard = mtx_;
    write_nolock(ptr, size);
}

file_stream_writer::file_stream_writer() = default;

file_stream_writer::file_stream_writer(file_stream&& stream)
    : stream_(std::move(stream))
{
}

file_stream_writer& file_stream_writer::operator=(file_stream&& stream)
{
    stream_ = std::move(stream);
    return *this;
}

system_console::~system_console()
{
    if (window_)
    {
        FreeConsole();
        PostMessage(window_, WM_CLOSE, 0U, 0L);
    }
    else
    {
        write_nolock("Stopped");
    }
}

system_console::system_console()
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

    /* constexpr auto construct_helper = [](auto& obj, auto... args) {
        std::destroy_at(&obj);
        std::construct_at(&obj, file_stream(args...));
    }; */

    in_  = file_stream("CONIN$", "r", stdin);
    out_ = file_stream("CONOUT$", "w", stdout);
    err_ = file_stream("CONOUT$", "w", stderr);

    FD_ASSERT(IsWindowUnicode(console_window) == TRUE);

    write_nolock("Started");
}

template <class M, class T = string>
static void _Write_log_line_nolock(file_stream_writer& w, const M& msg, const T& time = _Get_current_time())
{
#if 0
    w.write_nolock(time.data(), time.size());
    w.write_nolock(" - ", 3);
#else
    w.write_nolock("[", 1);
    w.write_nolock(time.data(), time.size());
    w.write_nolock("] ", 2);
#endif
    w.write_nolock(msg.data(), msg.size());
#ifdef _WIN32
    w.write_nolock("\n\r", 2);
#else
    w.write_nolock("\n", 1);
#endif
}

template <class M>
static void _Write_log_line(file_stream_writer& w, const M& msg)
{
    const auto time        = _Get_current_time();
    const lock_guard guard = w;
    _Write_log_line_nolock(w, msg, time);
}

void system_console::write_nolock(const string_view str)
{
    _Write_log_line_nolock(out_, str);
}

void system_console::write_nolock(const wstring_view wstr)
{
    _Write_log_line_nolock(out_, wstr);
}

void system_console::write(const string_view str)
{
    _Write_log_line(out_, str);
}

void system_console::write(const wstring_view wstr)
{
    _Write_log_line(out_, wstr);
}
