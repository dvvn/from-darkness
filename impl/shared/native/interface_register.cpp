#include "native/interface_register.h"
#include "algorithm/char.h"
#include "string/view.h"

namespace fd
{
inline namespace native
{
void* interface_register::get() const
{
    return create_();
}

char const* interface_register::name() const
{
    return name_;
}

interface_register* interface_register::next() const
{
    return next_;
}
} // namespace native

template <bool SkipVersion>
static interface_register* find_impl(
    interface_register* first, interface_register const* last, //
    char const* name, size_t const name_length)
{
    constexpr auto check_first_char = [](char const* name_last_char) {
        if constexpr (SkipVersion)
            return isdigit(*name_last_char);
        else
            return *name_last_char == '\0';
    };

    constexpr auto check_version = [](char const* str) {
        if constexpr (SkipVersion)
        {
            for (;; ++str)
            {
                auto const c = *str;
                if (isdigit(c))
                    continue;

                if (c == '\0')
                    break;

                return false;
            }
        }

        return true;
    };

    for (; first != last; first = first->next())
    {
        auto const src_name = first->name();

        if (!check_first_char(src_name + name_length))
            continue;
        if (memcmp(src_name, name, name_length) != 0)
            continue;
        if (!check_version(src_name + name_length + 1))
            continue;

        break;
    }
    return first;
}

enum class interface_register_find_flag : uint8_t
{
    unused,
    whole_check,
    duplicate_check
};

template <interface_register_find_flag Flag>
static interface_register* find_impl(
    interface_register* first, interface_register* last, //
    char const* name, size_t const name_length)
{
    using enum interface_register_find_flag;

    interface_register* found;

    auto const has_duplicate = [=, &found] {
        if (found->name()[name_length] != '\0')
        {
            auto const duplicate = find_impl<true>(found->next(), last, name, name_length);
            return duplicate != last;
        }
        return false;
    };

    if (Flag == whole_check || isdigit(name[name_length - 1]))
        found = find_impl<false>(first, last, name, name_length);
    else
        found = find_impl<true>(first, last, name, name_length);

    if constexpr (Flag == duplicate_check)
        if (found != last && has_duplicate())
            return last;

    return found;
}

interface_register* find_unique(
    interface_register* first, interface_register* last, //
    char const* name, size_t const name_length)
{
    return find_impl<interface_register_find_flag::duplicate_check>(first, last, name, name_length);
}

interface_register* find(
    interface_register* first, interface_register* last, //
    char const* name, size_t const name_length)
{
#if defined(_DEBUG) && 0
    constexpr auto flag = interface_register_find_flag::duplicate_check;
#else
    constexpr auto flag = interface_register_find_flag::unused;
#endif
    return find_impl<flag>(first, last, name, name_length);
}
}