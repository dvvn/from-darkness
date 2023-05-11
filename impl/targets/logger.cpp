#include <fd/logging/default.h>
#include <fd/logging/logger.h>

#include <fmt/chrono.h>
//

#include <windows.h>
//
#include <fcntl.h>
#include <io.h>
#include <tchar.h>

#include <cassert>

namespace fd
{
template <typename C>
struct system_console_logger : virtual abstract_logger<C>
{
    using typename abstract_logger<C>::pointer;
    using typename abstract_logger<C>::data_type;

    void do_write(pointer msg, size_t length) override
    {
        std::unreachable();
    }

    void write_before(data_type *d) override
    {
        std::unreachable();
    }

    void write_after(data_type *d) override
    {
        std::unreachable();
    }
};

static void do_write_before(auto *d)
{
    auto it = d->out();
    fmt::format_to(it, "[{:%H:%M:%S}]", std::chrono::system_clock::now().time_since_epoch());
    if constexpr (fmt::is_formattable<log_level, char>::value)
        fmt::format_to(it, " [{}]", d->level());
    d->push_back(' ');
}

void system_console_logger<char>::do_write(pointer msg, size_t length)
{
    (void)std::fwrite(msg, 1, length, stdout);
}

void system_console_logger<char>::write_before(data_type *d)
{
    do_write_before(d);
}

void system_console_logger<char>::write_after(data_type *d)
{
    d->push_back('\n');
}

//---

void system_console_logger<wchar_t>::do_write(pointer msg, size_t length)
{
#if 1
    assert(msg[length - 1] == L'\0');
    (void)std::fputws(msg, stdout);
    // std::perror("fputws()");
#else
    std::for_each(msg, msg + length, std::bind_back(fputwc, stdout));
#endif
}

void system_console_logger<wchar_t>::write_before(data_type *d)
{
    do_write_before(d);
}

void system_console_logger<wchar_t>::write_after(data_type *d)
{
    d->append(L"\n"); //"\n\0"
}

// template <typename It>
// static void remove_null_terminator(It it)
//{
//     auto &buff = fmt::detail::get_buffer<It::container_type>(it);
//     auto c     = *std::rbegin(fmt::detail::get_buffer<It::container_type>(it));
//     if (c == 0)
//         buff.resize(buff.size() - 1);
// }

static class : public logger_impl_wrapped<default_logger, system_console_logger>
{
    int current_mode_ = -1;
    int prev_mode_;

    void set_mode(int mode)
    {
        if (current_mode_ != mode)
        {
            prev_mode_    = _setmode(_fileno(stdout), mode);
            current_mode_ = mode;
        }
    }

    void reset_mode()
    {
        set_mode(prev_mode_);
    }

    void reset_mode_instant()
    {
        current_mode_ = _setmode(_fileno(stdout), prev_mode_);
    }

  protected:
    void init() override
    {
        SetConsoleTitle(_T("Title"));
        set_mode(_O_BINARY);
        reset_mode_instant();
        write<log_level::info>("Initialized");
    }

    void destroy() override
    {
        write<log_level::info>("Destroyed");
        std::destroy_at(this);
    }

    void do_write(abstract_logger<char>::pointer msg, size_t length) override
    {
        set_mode(/*_O_BINARY*/ _O_TEXT);
        system_console_logger<char>::do_write(msg, length);
    }

    void do_write(abstract_logger<wchar_t>::pointer msg, size_t length) override
    {
        set_mode(_O_U16TEXT);
        system_console_logger<wchar_t>::do_write(msg, length);
    }
} default_logger_impl;

default_logger *get_default_logger()
{
    return &default_logger_impl;
}
}