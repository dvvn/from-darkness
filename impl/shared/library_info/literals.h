#pragma once

#include "library_info/construct.h"
#include "string/static.h"

namespace fd::inline literals
{
#ifdef _DEBUG
inline library_info operator"" _dll(wchar_t const* name, size_t const length)
{
    return {
        wstring_view{name, length},
        L".dll"
    };
}
#else
template <constant_wstring Name>
library_info operator"" _dll()
{
    return {Name + L".dll"};
}
#endif

template <constant_string Name>
library_info operator"" _dll()
{
    return {Name + L".dll"};
}
} // namespace fd::inline literals