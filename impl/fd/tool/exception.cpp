#include "exception.h"
#include "string.h"

#include <Windows.h>

namespace fd
{
size_t basic_system_error::format(char **buffer, size_t buffer_length) const noexcept
{
    return FormatMessageA(
        // FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        code_,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<char *>(buffer),
        buffer_length,
        nullptr);
}

basic_system_error::basic_system_error(uintptr_t code) noexcept
    : code_(code)
{
}

basic_system_error::basic_system_error() noexcept
    : code_(GetLastError())
{
}

uintptr_t basic_system_error::code() const noexcept
{
    return code_;
}
}