#pragma once
#include "algorithm/char.h"
#include "library_info/function_getter.h"
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

#ifdef _DEBUG
inline constexpr char_table_wrapper<interface_version_char_table> interface_version_char;
#else
inline constexpr struct : char_table_wrapper<interface_version_char_table>
{
    bool operator()(char const c) const
    {
        if (c < 0)
            return false;
        return char_table_wrapper::operator()(c);
    }
} interface_version_char;
#endif

template <bool ContainsVersion>
interface_register* find_interface_register(
    interface_register* reg,                          //
    char const* const name, size_t const name_length, //
    bool_constant<ContainsVersion>) noexcept
{
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
} // namespace detail

inline void* find_interface(interface_register* const root_interface, string_view const interface_name) noexcept
{
    auto const name_data             = interface_name.data();
    auto const name_length           = interface_name.length();
    auto const name_contains_version = isdigit(interface_name.back());

    interface_register* found;

    using detail::find_interface_register;

    if (name_contains_version)
        found = find_interface_register(root_interface, name_data, name_length, true_type());
    else
    {
        found = find_interface_register(root_interface, name_data, name_length, false_type());
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

struct native_library_info::basic_interface_getter : basic_function_getter
{
    static safe_cast_lazy<void*> get(native::interface_register* const root_interface, string_view const interface_name)
    {
        return find_interface(root_interface, interface_name);
    }

    safe_cast_lazy<void*> get(string_view const name) const
    {
#ifdef _DEBUG
        static std::pair<void const*, native::interface_register*> cached;
        if (cached.first != this)
            cached = {this, root_interface()};
        return find_interface(cached.second, name);
#else
        return find_interface(root_interface(), name);
#endif
    }

  private:
    using basic_function_getter::create_interface;
    using basic_function_getter::find;
    native::interface_register* root_interface() const;
};

namespace detail
{

// template <class... T>
// requires(sizeof...(T) > 1)
// auto library_interface_getter<native_library_info>::find() const -> packed_objects<T...>
// {
//     auto const root = root_interface();
//     return {safe_cast_from(find_interface(root, native::interface_name<T>::value))...};
// }
} // namespace detail
} // namespace fd
