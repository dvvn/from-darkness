#pragma once
#include "algorithm/char.h"
#include "library_info/holder.h"
#include "native/interface_register.h"

#include <cassert>

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

namespace detail
{
class native_library_interface_getter
{
    native::interface_register* root_interface_;

  public:
    native_library_interface_getter(library_info const* linfo);

    safe_cast_lazy<void*> find(string_view const name) const
    {
        return find_interface(root_interface_, name);
    }
};
} // namespace detail
} // namespace fd
