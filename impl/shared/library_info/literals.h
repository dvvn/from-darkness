#pragma once

#include "library_info/construct.h"
#include "string/static.h"

namespace fd
{
template <basic_constant_string Name>
class named_library_info final : public library_info
{
    using object_getter = detail::library_object_getter<named_library_info>;

    static constexpr auto raw_name = Name + L".dll";

  public:
    named_library_info()
        : library_info{raw_name}
    {
    }

    object_getter obj() const
    {
        return {this};
    }
};

inline namespace literals
{
// #ifdef _DEBUG
// inline library_info operator"" _dll(wchar_t const* name, size_t const length)
//{
//     return {
//         wstring_view{name, length},
//         L".dll"
//     };
// }
// #else
// template <constant_wstring Name>
// library_info operator"" _dll()
//{
//     return {Name + L".dll"};
// }
// #endif

template <basic_constant_string Name>
auto operator"" _dll() -> named_library_info<Name>
{
    return {};
}
} // namespace literals
} // namespace fd