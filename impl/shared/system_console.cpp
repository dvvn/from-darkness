#include "system_console.h"

#include <corecrt_io.h>

#include <cassert>

namespace fd
{
namespace detail
{
system_console_mode_setter::system_console_mode_setter(int const descriptor, int const mode)
    : old_mode_{_setmode(descriptor, mode)}
{
    assert(old_mode_ != -1);
}
} // namespace detail

void system_console::write_wide(char const* buff, size_t const length) noexcept
{
    wchar_buffer_.assign(buff, buff + length);
    write(wchar_buffer_.data(), length);
}

system_console::~system_console()
{
    if (console_allocated_)
        FreeConsole();
}

system_console::system_console()
    : console_allocated_{false}
{
    if (!exists())
    {
        if (!AllocConsole())
        {
            assert(0 && "AllocConsole error!");
            return;
        }
        console_allocated_ = true;
    }

    out_ = GetStdHandle(STD_OUTPUT_HANDLE);
}

bool system_console::exists()
{
    return GetConsoleWindow() != INVALID_HANDLE_VALUE;
}

void system_console::write(wchar_t const* ptr, size_t const length)
{
#ifdef UNICODE
    DWORD written;
    WriteConsoleW(out_, ptr, length, &written, nullptr);
    assert(written == length);
#else
    FIXME
#endif
}

void system_console::write(char const* ptr, size_t const length)
{
#ifdef UNICODE
    write_wide(ptr, length);
#else
    DWORD written;
    WriteConsoleA(out_, ptr, length, &written, nullptr);
    assert(written == length);
#endif
}

void system_console::write(std::wstring_view const in_str)
{
#ifdef UNICODE
    write(in_str.data(), in_str.length());
#else
    FIXME
#endif
}

void system_console::write(std::string_view const in_str)
{
#ifdef UNICODE
    write_wide(in_str.data(), in_str.length());
#else
    write(in_str.data(), in_str.length());
#endif
}
} // namespace fd