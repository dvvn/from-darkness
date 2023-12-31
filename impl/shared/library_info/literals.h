#pragma once

#include "library_info/construct.h"
#include "string/static.h"

namespace fd
{
namespace detail
{
template <basic_constant_string Name>
struct named_library_info_base : type_identity<library_info>
{
};

template <basic_constant_string Name>
struct named_library_info_raw_name
{
    static constexpr auto value = Name + L".dll";
};

template <basic_constant_string Name>
class named_library_info final : public named_library_info_base<Name>::type
{
    using base_type     = typename named_library_info_base<Name>::type;
    using raw_name      = named_library_info_raw_name<Name>;
    using object_getter = library_object_getter<named_library_info>;

  public:
    named_library_info()
        : base_type({raw_name::value.data(), raw_name::value.size()})
    {
    }

    object_getter obj() const
    {
        return {this};
    }
};
} // namespace detail

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
auto operator"" _dll() -> detail::named_library_info<Name>
{
    return {};
}
} // namespace literals
} // namespace fd