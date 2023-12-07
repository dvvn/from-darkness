#pragma once

#include "string/static.h"
#include "library_info.h"

namespace fd
{
inline namespace literals
{
#ifdef _DEBUG
inline library_info operator"" _dll(wchar_t const* name, size_t length)
{
    return {
        wstring_view{name, length},
        library_info::extension::dll
    };
}
#else
template <static_wstring Name>
library_info operator"" _dll()
{
    return {Name + library_info::extension::dll};
}
#endif

template <static_string Name>
library_info operator"" _dll()
{
    return {Name + library_info::extension::dll};
}
#ifdef _DEBUG
inline native_library_info operator"" _dlln(wchar_t const* name, size_t length)
{
    return {
        wstring_view{name, length},
        library_info::extension::dll
    };
}
#else
template <static_wstring Name>
native_library_info operator"" _dlln()
{
    return {Name + library_info::extension::dll};
}
#endif
template <static_string Name>
native_library_info operator"" _dlln()
{
    return {Name + library_info::extension::dll};
}
} // namespace literals
}