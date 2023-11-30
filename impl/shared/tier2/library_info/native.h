#pragma once
#include "tier2/core.h"
#include "tier2/library_info/system.h"

#undef interface

namespace FD_TIER2(native)
{
class interface_register;
}

namespace FD_TIER(2)
{
struct native_library_info : system_library_info
{
    using interface_register = native::interface_register;

    using system_library_info::system_library_info;

    interface_register const& root_interface() const;
};

inline namespace literals
{
native_library_info operator"" _dlln(wchar_t const* name, size_t length);

template <static_string Name>
native_library_info operator"" _dlln()
{
    return {Name, native_library_info::extension_tag::dll};
}
} // namespace literals
} // namespace FD_TIER(2)