#pragma once
#include "algorithm/char.h"
#include "native/interface_register.h"
#include "library_info_native.h"

#include <cassert>

namespace fd
{
namespace detail
{
struct interface_version_table : isdigit_table
{
    constexpr interface_version_table()
    {
        operator[]('_') = true;
    }
};

inline constexpr char_table_wrapper<interface_version_table> interface_version;
} // namespace detail

inline void* native_library_info::find(interface_string const name) const
{
    auto const name_length = name.length();

    auto const find = [name_length, name_first = ubegin(name)]<bool ContainsVersion>(std::bool_constant<ContainsVersion>, native::interface_register* reg) {
        for (; reg != nullptr; reg = reg->next())
        {
            auto const reg_name      = reg->name();
            auto const reg_name_last = reg_name + name_length;

            if constexpr (!ContainsVersion)
            {
                if (!detail::interface_version(*reg_name_last))
                    goto _SKIP_BREAK;
            }
            else
            {
                if (*reg_name_last != '\0')
                    goto _SKIP_BREAK;
            }
            if (!std::equal(reg_name, reg_name_last, name_first))
                goto _SKIP_BREAK;
            if constexpr (!ContainsVersion)
            {
                for (auto version_start = reg_name_last + 1; *version_start != '\0'; ++version_start)
                {
                    if (!detail::interface_version(*version_start))
                        goto _SKIP_BREAK;
                }
            }
            break;
        _SKIP_BREAK:
            (void)0;
        }
        return reg;
    };

    native::interface_register* found;
    auto const root_ifc              = root_interface();
    auto const name_contains_version = isdigit(name.back());
    if (name_contains_version)
        found = find(std::true_type(), root_ifc);
    else
    {
        found = find(std::false_type(), root_ifc);
#ifdef _DEBUG
        if (found && found->name()[name_length] != '\0')
        {
            auto const found_again = find(std::false_type(), found->next());
            if (found_again)
            {
                assert(0 && "found multiple iterfaces for given name");
                return nullptr;
            }
        }
#endif
    }
    assert(found);
    return found->get();
}
}