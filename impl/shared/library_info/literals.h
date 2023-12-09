#pragma once

#include "library_info/construct.h"
#include "string/static.h"

namespace fd
{
inline namespace literals
{
#ifdef _DEBUG
inline library_info operator"" _dll(wchar_t const* name, size_t length)
{
    return {
        wstring_view{name, length},
        L".dll"
    };
}
#else
template <static_wstring Name>
library_info operator"" _dll()
{
    return {Name + L".dll"};
}
#endif

template <static_string Name>
library_info operator"" _dll()
{
    return {Name + L".dll"};
}
} // namespace literals
} // namespace fd