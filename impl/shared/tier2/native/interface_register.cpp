#include "tier0/iterator/unwrap.h"
#include "tier1/algorithm/char.h"
#include "tier1/functional/bind.h"
#include "tier2/native/interface_register.h"

#include <cassert>

template <>
struct std::iterator_traits<fd::native::interface_register::iterator> : std::iterator_traits<fd::native::interface_register const*>
{
    using iterator_category = forward_iterator_tag;
    using difference_type   = size_t;
};

namespace FD_TIER2(native)
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

auto interface_register::iterator::operator++(int) -> iterator
{
    assert(current_ != nullptr);
    auto const curr = current_;
    current_        = current_->next_;
    return curr;
}

auto interface_register::iterator::operator++() -> iterator&
{
    assert(current_ != nullptr);
    current_ = current_->next_;
    return *this;
}

interface_register const& interface_register::iterator::operator*() const
{
    assert(current_ != nullptr);
    return *current_;
}

interface_register const* interface_register::iterator::operator->() const
{
    assert(current_ != nullptr);
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
    auto table                 = isdigit_table;
    char_table_get(table, '_') = true;
    return table;
}();

static bool check_interface_version_char(char c)
{
    return char_table_get(interface_version_chars, c);
}

static bool check_interface_version(char const* version_start)
{
    for (; *version_start != '\0'; ++version_start)
    {
        if (!check_interface_version_char(*version_start))
            return false;
    }
    return true;
}

auto interface_register::find(string_view const name) const -> iterator
{
    return find(name, isdigit(name.back()));
}

auto interface_register::find(string_view name, bool const name_contains_version) const -> iterator
{
    auto const name_length = name.length();

    auto const compare = [name_length, name_first = ubegin(name), name_last = uend(name)] //
        <bool ContainsVersion>                                                            //
        (std::bool_constant<ContainsVersion>, interface_register const& reg) {
            auto const reg_name_end = reg.name_ + name_length;

            if constexpr (!ContainsVersion)
            {
                if (!check_interface_version_char(*reg_name_end))
                    return false;
            }
            else
            {
                if (*reg_name_end != '\0')
                    return false;
            }
            if (!std::equal(reg.name_, reg_name_end, name_first, name_last))
                return false;
            if constexpr (!ContainsVersion)
            {
                if (!check_interface_version(reg_name_end + 1))
                    return false;
            }

            return true;
        };

    iterator const first{this}, last{nullptr};

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

interface_register::iterator begin(interface_register const& reg)
{
    return &reg;
}

interface_register::iterator end(interface_register const& reg)
{
    (void)reg;
    return nullptr;
}

basic_interface::basic_interface(interface_register::iterator current, string_view name, void** ptr)
{
    *ptr = current->find(name)->try_get();
}

void* get(interface_register const& reg, string_view const name)
{
    auto target = reg.find(name);
#ifdef _DEBUG
    return target->try_get();
#else
    return target->get();
#endif
}
}