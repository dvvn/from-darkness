#include "native/interface_register.h"
#include "algorithm/char.h"
#include "functional/bind.h"
#include "iterator/unwrap.h"
#include "string/view.h"

#include <cassert>

template <>
struct std::iterator_traits<fd::native::interface_register::iterator> : std::iterator_traits<fd::native::interface_register const*>
{
    using iterator_category = forward_iterator_tag;
    using difference_type   = size_t;
};

namespace fd::native
{
void* interface_register::get() const
{
    return create_();
}

char const* interface_register::name() const
{
    return name_;
}

void* interface_register::try_get() const
{
    return this ? get() : nullptr;
}

interface_register::iterator::iterator()
{
    (void)this;
}

interface_register::iterator::iterator(interface_register const* current)
    : current_{current}
{
}

interface_register::iterator& interface_register::iterator::operator++(int)
{
    current_ = current_->next_;
    return *this;
}

interface_register::iterator interface_register::iterator::operator++() const
{
    return current_->next_;
}

interface_register const& interface_register::iterator::operator*() const
{
    return *current_;
}

interface_register const* interface_register::iterator::operator->() const
{
    return current_;
}

interface_register::iterator::operator bool() const
{
    return current_ != nullptr;
}

bool interface_register::iterator::operator==(iterator const& other) const
{
    return current_ == other.current_;
}

inline constexpr auto interface_version_chars = [] {
    auto table                         = detail::isdigit_table;
    detail::char_table_get(table, '_') = true;
    return table;
}();

static bool check_interface_version_char(char const* str_start)
{
    return detail::char_table_get(interface_version_chars, *str_start);
}

static bool check_interface_version(char const* version_start)
{
    for (; *version_start != '\0'; ++version_start)
    {
        if (!check_interface_version_char(version_start))
            return false;
    }
    return true;
}

interface_register::iterator find(interface_register const* current, char const* name, size_t const name_length)
{
    return find(current, name, name_length, isdigit(name[name_length - 1]));
}

interface_register::iterator find(interface_register const* current, char const* name, size_t const name_length, bool const name_contains_version)
{
    auto const compare = [=]<bool ContainsVersion>(std::bool_constant<ContainsVersion>, interface_register const& reg) {
        if constexpr (ContainsVersion)
        {
            if (reg.name_[name_length] != '\0')
                return false;
        }
        else
        {
            if (!check_interface_version_char(reg.name_ + name_length))
                return false;
        }
        if (memcmp(reg.name_, name, name_length) != 0)
            return false;

        if constexpr (!ContainsVersion)
            if (!check_interface_version(reg.name_ + name_length + 1))
                return false;

        return true;
    };

    interface_register::iterator const first{current}, last{nullptr};

    if (name_contains_version)
        return std::find_if(first, last, bind_front(compare, std::true_type()));

    auto const found = std::find_if(first, last, bind_front(compare, std::false_type()));
#ifdef _DEBUG
    if (found != last && found->name_[name_length] != '\0')
    {
        auto const found_again = std::find_if(std::next(found), last, bind_front(compare, std::false_type()));
        if (found_again != last)
        {
            assert(0 && "found multiple iterfaces for given name");
            return last;
        }
    }
#endif

    return found;
}

interface_register::iterator begin(interface_register const* reg)
{
    return reg;
}

interface_register::iterator end(interface_register const* reg)
{
    (void)reg;
    return nullptr;
}
}