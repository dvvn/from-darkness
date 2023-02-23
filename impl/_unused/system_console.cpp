#include <fd/assert.h>
#include <fd/format.h>
#include <fd/system_console.h>

#include <fcntl.h>
#include <io.h>

#include <array>
#include <chrono>
#include <cstdio>

namespace fd
{
file_stream::~file_stream()
{
    if (!redirected_ || !stream_)
        return;
    _fclose_nolock(stream_);
}

file_stream::file_stream()
    : redirected_(false)
    , stream_(nullptr)
{
}

file_stream::file_stream(FILE* stream)
    : redirected_(false)
    , stream_(stream)
{
}

file_stream::file_stream(const char* fileName, const char* mode, FILE* oldStream)
    : redirected_(true)
{
    [[maybe_unused]] const auto err = freopen_s(&stream_, fileName, mode, oldStream);
    FD_ASSERT(err == NULL);
}

file_stream::file_stream(file_stream&& other) noexcept
{
    *this         = other;
    other.stream_ = nullptr;
}

file_stream& file_stream::operator=(file_stream&& other) noexcept
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

static string _get_current_time()
{
    using namespace std::chrono;
    using clock = system_clock;

#if 1
    return format("{:%T}", current_zone()->to_local(clock::now()).time_since_epoch());
#else
    const auto current_time_point = clock::now();
    const auto current_time       = clock::to_time_t(current_time_point);
    tm         current_localtime;

    localtime_s(&current_localtime, std::addressof(current_time));

    const auto current_time_since_epoch = current_time_point.time_since_epoch();
    const auto current_milliseconds     = duration_cast<milliseconds>(current_time_since_epoch).count() % 1000;
    const auto current_microseconds     = duration_cast<microseconds>(current_time_since_epoch).count() % 1000;

    std::ostringstream ss;
    ss << std::setfill('0') << std::put_time(&current_localtime, "%T") << ' ' << std::setw(3) << current_milliseconds << '.' << std::setw(3) << current_microseconds;
    return std::move(ss).str();
#endif
}

#define PUTC_ASSERT(_RESULT_) FD_ASSERT(_RESULT_ != WEOF, errno == EILSEQ ? "Encoding error in putc." : "I/O error in putc.")

console_reader::console_reader() = default;

void console_reader::set(file_stream&& stream)
{
    stream_ = std::move(stream);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void console_writer::write_nolock(const wchar_t* ptr, const size_t size)
{
    // [[maybe_unused]] const auto ok = std::fputws(ptr, stream_);
    // putc_assert(ok);

#ifdef _WIN32
    const auto h = _fileno(stream_);
    if (_isatty(h))
    {
        DWORD written;
        // ReSharper disable once CppRedundantCastExpression
        if (WriteConsoleW(reinterpret_cast<void*>(_get_osfhandle(h)), ptr, static_cast<DWORD>(size), &written, nullptr))
            return;

        // Fallback to fwrite on failure. It can happen if the output has been
        // redirected to NUL.
    }
#endif
    _fwrite_nolock(ptr, sizeof(wchar_t), size, stream_);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void console_writer::write_nolock(const char* ptr, const size_t size)
{
    /* const auto wstr = utf_convert<wchar_t>(string_view(ptr, size));
     write(wstr.data(), wstr.size()); */
    _fwrite_nolock(ptr, sizeof(char), size, stream_);
}

void console_writer::write(const wchar_t* ptr, const size_t size)
{
    const std::lock_guard guard(mtx_);
    write_nolock(ptr, size);
}

void console_writer::write(const char* ptr, const size_t size)
{
    const std::lock_guard guard(mtx_);
    write_nolock(ptr, size);
}

void console_writer::lock()
{
    mtx_.lock();
}

void console_writer::unlock()
{
    mtx_.unlock();
}

console_writer::console_writer() = default;

void console_writer::set(file_stream&& stream)
{
    stream_ = std::move(stream);
}

template <class M, class T = string>
static void _write_log_line_nolock(console_writer& w, const M& msg, const T& time = _get_current_time())
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
static void _write_log_line(console_writer& w, const M& msg)
{
    const auto            time = _get_current_time();
    const std::lock_guard guard(w);
    _write_log_line_nolock(w, msg, time);
}

console_writer_front::~console_writer_front()
{
    if (writer_)
        writer_->unlock();
}

console_writer_front::console_writer_front(console_writer& writer)
    : writer_(&writer)
{
    writer_->lock();
}

console_writer_front& console_writer_front::operator=(console_writer_front&& other)
{
    writer_       = other.writer_;
    other.writer_ = 0;
    return *this;
}

void console_writer_front::operator()(const string_view msg) const
{
    _write_log_line_nolock(*writer_, msg);
}

void console_writer_front::operator()(const wstring_view msg) const
{
    _write_log_line_nolock(*writer_, msg);
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
        _write_log_line_nolock(out_, "Stopped"sv);
    }
}

system_console::system_console()
{
    auto consoleWindow = GetConsoleWindow();
    if (!consoleWindow)
    {
        const auto consoleAllocated = AllocConsole();
        FD_ASSERT(consoleAllocated, "Unable to allocate the console!");

        consoleWindow = GetConsoleWindow();
        FD_ASSERT(consoleWindow, "Unable to get the console window");

        // const auto window_title_set = SetConsoleTitleW(nstd::winapi::current_module()->FullDllName.Buffer);
        // FD_ASSERT(window_title_set, "Unable set console title");

        window_ = consoleWindow;
    }

    /* constexpr auto construct_helper = [](auto& obj, auto... args) {
        std::destroy_at(&obj);
        std::construct_at(&obj, file_stream(args...));
    }; */

    in_.set({ "CONIN$", "r", stdin });
    out_.set({ "CONOUT$", "w", stdout });
    err_.set({ "CONOUT$", "w", stderr });

    FD_ASSERT(IsWindowUnicode(consoleWindow) == TRUE);

    _write_log_line_nolock(out_, "Started"sv);
}

console_writer_front system_console::out()
{
    return out_;
}

console_writer_front system_console::err()
{
    return err_;
}
}