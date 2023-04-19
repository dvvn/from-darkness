#include <fd/logging/system_console.h>

#include <fmt/chrono.h>

#include <cassert>

namespace fd
{
static void do_write_before(auto *d)
{
    auto it = d->out();
    fmt::format_to(it, "[{:%H:%M:%S}]", std::chrono::system_clock::now().time_since_epoch());
    if constexpr (fmt::is_formattable<log_level, char>::value)
        fmt::format_to(it, " [{}]", d->level());
    d->push_back(' ');
}

//---

void system_console<char>::do_write(pointer msg, size_t length)
{
    (void)std::fwrite(msg, 1, length, stdout);
}

void system_console<char>::write_before(data_type *d)
{
    do_write_before(d);
}

void system_console<char>::write_after(data_type *d)
{
    d->push_back('\n');
}

//---

void system_console<wchar_t>::do_write(pointer msg, size_t length)
{
#if 1
    assert(msg[length - 1] == L'\0');
    (void)std::fputws(msg, stdout);
    // std::perror("fputws()");
#else
    std::for_each(msg, msg + length, std::bind_back(fputwc, stdout));
#endif
}

void system_console<wchar_t>::write_before(data_type *d)
{
    do_write_before(d);
}

void system_console<wchar_t>::write_after(data_type *d)
{
    d->append(L"\n"); //"\n\0"
}
} // namespace fd