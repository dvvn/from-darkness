﻿#include "library_info/basic_interface_getter.h"

#include <cassert>
#include <cctype>

namespace fd
{
namespace native
{
namespace detail
{
// XXXX_123
// XXXXV123
// XXXX123
static bool interface_version_char(char const c) noexcept
{
    switch (c)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'V':
    case '_':
        return true;
    default:
        return false;
    }
}

template <bool ContainsVersion>
static interface_register* find_interface_register(
    interface_register* reg,                          //
    char const* const name, size_t const name_length, //
    std::bool_constant<ContainsVersion>) noexcept
{
    for (; reg != nullptr; reg = reg->next())
    {
        auto const reg_name_first = reg->name();
        auto const reg_name_last  = reg_name_first + name_length;

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
        if (!std::equal(reg_name_first, reg_name_last, name))
        {
            continue;
        }
        if constexpr (!ContainsVersion)
        {
            auto version_first = reg_name_last + 1;
            if (!interface_version_char(*version_first))
                continue;
#ifdef _MSC_VER
            // ReSharper disable once CppRedundantControlFlowJump
            goto _CHECK_DIGIT;
#endif
        _CHECK_DIGIT:
            ++version_first;
            auto const version_start_chr = *version_first;
            if (version_start_chr != '\0')
            {
                if (!std::isdigit(version_start_chr))
                    continue;
                goto _CHECK_DIGIT;
            }
        }
        break;
    }
    return reg;
}
} // namespace detail

static void* find_interface(interface_register* const root_interface, std::string_view const interface_name) noexcept
{
    auto const name_data             = interface_name.data();
    auto const name_length           = interface_name.length();
    auto const name_contains_version = std::isdigit(interface_name.back());

    interface_register* found;

    using detail::find_interface_register;

    if (name_contains_version)
        found = find_interface_register(root_interface, name_data, name_length, std::true_type());
    else
    {
        found = find_interface_register(root_interface, name_data, name_length, std::false_type());
#ifdef _DEBUG
        if (found && found->name()[name_length] != '\0')
        {
            auto const found_again = find_interface_register(found->next(), name_data, name_length, std::false_type());
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
native_library_interface_getter::native_library_interface_getter(library_info const* linfo)
    : root_interface_{get_root_interface(linfo)}
{
}

safe_cast_lazy<void*> native_library_interface_getter::find(std::string_view const name) const
{
    return find_interface(root_interface_, name);
}
} // namespace detail
} // namespace fd