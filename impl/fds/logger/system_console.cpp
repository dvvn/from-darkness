module;
#include <fds/core/assert.h>
#include <fds/core/object.h>

#include <Windows.h>
#include <fcntl.h>
#include <io.h>

#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>
#include <time.h>

module fds.logger.system_console;
import fds.convert_to;

class stream_descriptor
{
    int descriptor_;

  public:
    ~stream_descriptor()
    {
    }

    stream_descriptor() = default;

    stream_descriptor(FILE* const stream)
        : descriptor_(_fileno(stream))
    {
        // If stdout or stderr is not associated with an output stream (for example, in a Windows application without a console window),
        // the file descriptor returned is -2. In previous versions, the file descriptor returned was -1
        assert(descriptor_ >= 0);
    }

    operator int() const
    {
        return descriptor_;
    }
};

class stream_mode_changer
{
    stream_descriptor descriptor_;
    int prev_mode_ = -1, curr_mode_ = -1;

    int set_mode_impl(const int mode) const
    {
        // If you write data to a file stream, explicitly flush the code by using fflush before you use _setmode to change the mode
        const auto prev_mode = _setmode(descriptor_, mode);
        assert(prev_mode != -1 && "Unable to change mode");
        return prev_mode;
    }

    stream_mode_changer(const stream_mode_changer&)            = default;
    stream_mode_changer& operator=(const stream_mode_changer&) = default;

  public:
    ~stream_mode_changer()
    {
        if (prev_mode_ != -1)
            set_mode_impl(prev_mode_);
    }

    stream_mode_changer() = default;

    stream_mode_changer(FILE* const stream)
        : descriptor_(stream)
    {
    }

    stream_mode_changer(stream_mode_changer&& other)
    {
        *this            = other;
        other.prev_mode_ = -1;
    }

    stream_mode_changer& operator=(stream_mode_changer&& other)
    {
        const auto copy = *this;
        *this           = other;
        other           = copy;
        return *this;
    }

    bool set(const int mode)
    {
        // also may be done by 'fwide'
        if (curr_mode_ == mode)
            return false;
        const auto prev_mode = set_mode_impl(mode);
        curr_mode_           = mode;
        if (prev_mode_ == -1)
            prev_mode_ = prev_mode;
        return true;
    }

    template <typename C>
    bool set();

    template <>
    bool set<char>()
    {
        return set(_O_TEXT);
    }

    template <>
    bool set<wchar_t>()
    {
        return set(_O_U16TEXT);
    }

    template <>
    bool set<char8_t>()
    {
        return set(_O_U8TEXT);
    }

    stream_mode_changer release()
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

    file_stream(const file_stream&)            = default;
    file_stream& operator=(const file_stream&) = default;

  public:
    ~file_stream()
    {
        if (!redirected_ || !stream_)
            return;
        fclose(stream_);
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
        assert(err == NULL);
    }

    file_stream(file_stream&& other)
    {
        *this         = other;
        other.stream_ = nullptr;
    }

    file_stream& operator=(file_stream&& other)
    {
        const auto copy = *this;
        *this           = other;
        other           = copy;
        return *this;
    }

    operator FILE*() const
    {
        return stream_;
    }
};

class time_buff final
{
    std::string buff_;

  public:
    template <size_t S = std::char_traits<char>::length("01:13:03 224.095")>
    constexpr size_t max_size() const
    {
        return S;
    }

    time_buff()
    {
        using namespace std::chrono;
        using clock = system_clock;

        const auto current_time_point = clock::now();
        const auto current_time       = clock::to_time_t(current_time_point);
        tm current_localtime;

        localtime_s(&current_localtime, std::addressof(current_time));

        const auto current_time_since_epoch = current_time_point.time_since_epoch();
        const auto current_milliseconds     = duration_cast<milliseconds>(current_time_since_epoch).count() % 1000;
        const auto current_microseconds     = duration_cast<microseconds>(current_time_since_epoch).count() % 1000;

        buff_.reserve(this->max_size());
        std::ostringstream sbuff(std::move(buff_));
        sbuff << std::setfill('0') << std::put_time(&current_localtime, "%T") << ' ' << std::setw(3) << current_milliseconds << '.' << std::setw(3) << current_microseconds;
        buff_ = std::move(sbuff).str();
    }

    operator std::string_view() const
    {
        return buff_;
    }

    operator std::string() &&
    {
        return buff_;
    }
};

#define putc_assert(_RESULT_) FDS_ASSERT(_RESULT_ != WEOF, errno == EILSEQ ? "Encoding error in putc." : "I/O error in putc.")

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
    stream_mode_changer changer_;
    std::mutex mtx_;

    template <typename Ch>
    void write_nolock(const std::basic_string_view<char, Ch> text)
    {
        changer_.set<char>();
        [[maybe_unused]] const auto written = _fwrite_nolock(text.data(), 1, text.size(), stream_);
        assert(written == text.size());
    }

    void write_nolock(const char chr)
    {
        changer_.set<char>();
        [[maybe_unused]] const auto ok = _putc_nolock(chr, stream_);
        putc_assert(ok);
    }

    template <typename Ch>
    void write_nolock(const std::basic_string_view<wchar_t, Ch> text)
    {
        // changer_.set<wchar_t>();
        //  for (const auto chr : text)
        //  {
        //      [[maybe_unused]] const auto ok = _putwc_nolock(chr, stream_);
        //      FDS_ASSERT(ok != WEOF, errno == EILSEQ ? "Encoding error in fputwc." : "I/O error in fputwc.");
        //  }

        // idk how to made it works, here is temp gap
        const auto u8text = fds::convert_to<char>(text);
        FDS_ASSERT(u8text.size() == text.size(), "Unicode not supported");
        write_nolock(u8text);
    }

    template <typename C, typename Ch>
    void write_nolock(const std::basic_string<C, Ch>& text)
    {
        write_nolock(std::basic_string_view<C, Ch>(text));
    }

    template <typename C, size_t S>
    void write_nolock(const C (&text)[S])
    {
        const auto null_terminated = text[S - 1] == static_cast<C>(0);
        const auto text_size       = null_terminated ? S - 1 : S;
        write_nolock(std::basic_string_view<C>(text, text + text_size));
    }

  public:
    writer() = default;

    writer(file_stream&& stream)
        : stream_(std::move(stream))
        , changer_(stream_)
    {
    }

    writer& operator=(file_stream&& stream)
    {
        stream_  = std::move(stream);
        changer_ = static_cast<FILE*>(stream_);
        return *this;
    }

    template <typename C, typename Ch, class Locker>
    void operator()(const std::basic_string_view<C, Ch> text, const Locker locker)
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

        const auto lock = locker(mtx_);
        write_nolock(time);
        if constexpr (!can_reuse_buff)
        {
            constexpr char dash[] = { ' ', '-', ' ', 0 };
            write_nolock(dash);
        }
        write_nolock(text);
        write_nolock('\n');
    }
};

constexpr auto real_locker = []<class Mtx>(Mtx& mtx) {
    return std::scoped_lock(mtx);
};

constexpr auto fake_locker = []<class Mtx>(Mtx&) {
    return std::false_type();
};

class console_writer_impl : public console_writer
{
    reader in_;
    writer out_;
    writer err_;

    HWND window_ = nullptr;

  public:
    void operator()(const std::string_view str) override
    {
        out_(str, real_locker);
    }

    void operator()(const std::wstring_view wstr) override
    {
        out_(wstr, real_locker);
    }

    ~console_writer_impl() override
    {
        // fds_assert_remove_handler(this->id());

        if (window_)
        {
            FreeConsole();
            PostMessage(window_, WM_CLOSE, 0U, 0L);
        }
        else
        {
            out_(std::string_view("Stopped"), fake_locker);
        }
    }

    console_writer_impl()
    {
        auto console_window = GetConsoleWindow();
        if (!console_window)
        {
            const auto console_allocated = AllocConsole();
            FDS_ASSERT(console_allocated, "Unable to allocate the console!");

            console_window = GetConsoleWindow();
            FDS_ASSERT(console_window, "Unable to get the console window");

            /* in_ = file_stream("CONIN$", "r", stdin);
            out_ = file_stream("CONOUT$", "w", stdout);
            err_ = file_stream("CONOUT$", "w", stderr); */

            // const auto window_title_set = SetConsoleTitleW(nstd::winapi::current_module()->FullDllName.Buffer);
            // FDS_ASSERT(window_title_set, "Unable set console title");

            window_ = console_window;
        }

        in_  = file_stream("CONIN$", "r", stdin);
        out_ = file_stream("CONOUT$", "w", stdout);
        err_ = file_stream("CONOUT$", "w", stderr);

        FDS_ASSERT(IsWindowUnicode(console_window) == TRUE);
        // fds_assert_add_handler(this);

        out_(std::string_view("Started"), fake_locker);
    }
};

FDS_OBJECT_BIND_TYPE(system_console_writer, console_writer_impl);
