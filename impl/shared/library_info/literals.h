#pragma once

#include "library_info/construct.h"
#include "string/static.h"
#include "concepts.h"

namespace fd
{
template <basic_constant_string Name>
class named_library_info;

inline namespace literals
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

template <basic_constant_string Name>
#ifdef _DEBUG
requires complete<named_library_info<Name>>
#endif
named_library_info<Name> operator"" _dll()
{
    return {};
}
} // namespace literals
} // namespace fd