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

reader::reader() = default;

reader& reader::operator=(file_stream&& stream)
{
    stream_ = std::move(stream);
    return *this;
}

void writer::write(const wchar_t* ptr, const size_t size)
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

void writer::write(const char* ptr, const size_t size)
{
    /* const auto wstr = utf_convert<wchar_t>(string_view(ptr, size));
     write(wstr.data(), wstr.size()); */
    _fwrite_nolock(ptr, sizeof(char), size, stream_);
}

writer::writer() = default;

writer::writer(file_stream&& stream)
    : stream_(std::move(stream))
{
}

writer& writer::operator=(file_stream&& stream)
{
    stream_ = std::move(stream);
    return *this;
}

void logs_writer::write_unsafe(const string_view text)
{
    write_impl(_Get_current_time(), text);
}

void logs_writer::write_unsafe(const wstring_view text)
{
    write_impl(_Get_current_time(), text);
}

void logs_writer::write(const string_view text)
{
    const auto time        = _Get_current_time();
    const lock_guard guard = mtx_;
    write_impl(time, text);
}

void logs_writer::write(const wstring_view text)
{
    const auto time        = _Get_current_time();
    const lock_guard guard = mtx_;
    write_impl(time, text);
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
        out_.write_unsafe("Stopped");
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

    out_.write_unsafe("Started");
}

void system_console::write(const string_view str)
{
    out_.write(str);
}

void system_console::write(const wstring_view wstr)
{
    out_.write(wstr);
}
