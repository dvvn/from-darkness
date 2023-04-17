#include <fd/formatting/time.h>
#include <fd/logging/system_console.h>

#include <cassert>

namespace fd
{
void system_console::write(pointer msg, size_t length)
{
    (void)std::fwrite(msg, 1, length, stdout);
}

void system_console::write(wpointer msg, size_t length)
{
#if 1
    assert((msg[length - 1]) == L'\0');
    (void)std::fputws(msg, stdout);
    // std::perror("fputws()");

#else
    std::for_each(msg, msg + length, std::bind_back(fputwc, stdout));
#endif
}

static void do_write_before(auto it)
{
    format_time(it);
    ++*it = ' ';
}

void system_console::write_before(itr it) const
{
    do_write_before(it);
}

void system_console::write_before(witr it) const
{
    do_write_before(it);
}

void system_console::write_after(itr it) const
{
    ++*it = '\n';
    //++*it = '\0';
}

void system_console::write_after(witr it) const
{
    ++*it = '\n';
    ++*it = '\0';
}

} // namespace fd