#pragma once
#include "algorithm/char.h"
#include "library_info/function.h"
#include "library_info/interface_name.h"
#include "native/interface_register.h"

#include <cassert>

#undef interface

namespace fd
{
namespace native
{
namespace detail
{
// XXXX_123
// XXXXV123
// XXXX123
struct interface_version_char_table : basic_char_table<bool>
{
    constexpr interface_version_char_table()
        : basic_char_table{isdigit.table()}
    {
        set('_', true);
        set('V', true);
    }
};

inline constexpr char_table_wrapper<interface_version_char_table> interface_version_char;
} // namespace detail

template <bool ContainsVersion>
interface_register* find_interface_register(
    interface_register* reg,                          //
    char const* const name, size_t const name_length, //
    bool_constant<ContainsVersion>)
{
    using detail::interface_version_char;

    for (; reg != nullptr; reg = reg->next())
    {
        auto const reg_name      = reg->name();
        auto const reg_name_last = reg_name + name_length;

        if (!ContainsVersion)
        {
            if (!interface_version_char(*reg_name_last))
                continue;
        }
        else
        {
            if (*reg_name_last != '\0')
                continue;
        }
        if (!std::equal(reg_name, reg_name_last, name))
        {
            continue;
        }
        if constexpr (!ContainsVersion)
        {
            auto version_start = reg_name_last + 1;
            if (interface_version_char(*version_start))
                goto _CHECK_DIGIT; // MSVC fails without this
            (void)0;
        _CHECK_DIGIT:
            ++version_start;
            auto const version_start_chr = *version_start;
            if (version_start_chr != '\0')
            {
                if (!isdigit(version_start_chr))
                    continue;
                goto _CHECK_DIGIT;
            }
        }
        break;
    }
    return reg;
}

inline void* find_interface(interface_register* const root_ifc, string_view const name)
{
    auto const name_data             = name.data();
    auto const name_length           = name.length();
    auto const name_contains_version = isdigit(name.back());

    interface_register* found;

    if (name_contains_version)
        found = find_interface_register(root_ifc, name_data, name_length, true_type());
    else
    {
        found = find_interface_register(root_ifc, name_data, name_length, false_type());
#ifdef _DEBUG
        if (found && found->name()[name_length] != '\0')
        {
            auto const found_again = find_interface_register(found->next(), name_data, name_length, false_type());
            if (found_again)
            {
                assert(0 && "found multiple iterfaces for given name");
                return nullptr;
            }
        }
#endif
    }
    assert(found != nullptr);
    return found->get();
}
} // namespace native

inline void* native_library_info::basic_interface_getter::find(string_view const name) const
{
    return find_interface(root_interface(), name);
}

// template <typename... Name>
// requires(sizeof...(Name) > 1)
// array<void*, sizeof...(Name)> native_library_info::basic_interface_getter::find(Name const&... name) const
//{
//     auto const root = root_interface();
//     return {find_interface(root, name)...};
// }

template <class T>
T* native_library_info::basic_interface_getter::find() const
{
    return safe_cast_from(find_interface(root_interface(), native::interface_name<T>::value));
}

template <class... T>
requires(sizeof...(T) > 1)
auto native_library_info::basic_interface_getter::find() const -> packed_objects<T...>
{
    auto const root = root_interface();
    return {safe_cast_from(find_interface(root, native::interface_name<T>::value))...};
}

} // namespace fd