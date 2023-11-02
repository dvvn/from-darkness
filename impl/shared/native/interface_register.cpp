#include "native/interface_register.h"
#include "functional/ignore.h"
#include "string/char.h"
#include "string/view.h"

#include <cassert>

namespace fd
{
static bool all_digits(char const* ptr)
{
    for (auto c = *ptr; c != '\0'; c = *++ptr)
    {
        if (!isdigit(c))
            return false;
    }
    return true;
}

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

template <bool SkipVersion>
static interface_register* find(
    interface_register* first, interface_register* last, //
    char const* name, size_t const length)
{
    for (; first != last; first = first->next())
    {
        auto const src_name      = first->name();
        auto const src_name_back = src_name[length];

        if (SkipVersion ? !isdigit(src_name_back) : src_name_back != '\0')
            continue;

        if (memcmp(src_name, name, length) != 0)
            continue;

        if constexpr (SkipVersion)
        {
            if (!all_digits(src_name + length + 1))
                continue;
        }

        return first;
    }
    return last;
}

#if 0
template <bool Partial>
class found_interface_register;

class basic_found_interface_register
{
    interface_register* holder_;
    size_t input_string_length_;

    template <bool>
    friend class found_interface_register;

  public:
    // ReSharper disable once CppPossiblyUninitializedMember
    basic_found_interface_register(interface_register* holder)
        : holder_(holder)
    {
    }

    interface_register* operator->() const
    {
        return holder_;
    }

    explicit operator bool() const
    {
        return holder_;
    }

    size_t input_string_length() const
    {
        return input_string_length_;
    }

    basic_found_interface_register& set_input_string_length(size_t const input_string_length)
    {
        input_string_length_ = input_string_length;
        return *this;
    }

    string_view name() const
    {
        auto n = holder_->name();
        return {n, n + input_string_length_};
    }
};

template <>
class found_interface_register<true> : public basic_found_interface_register
{
    bool exact_;

  public:
    using basic_found_interface_register::basic_found_interface_register;

    found_interface_register(interface_register* holder, bool const exact)
        : basic_found_interface_register(holder)
        , exact_(exact)
    {
    }

    bool exact() const
    {
        return exact_;
    }

    string_view full_name() const
    {
        auto n   = holder_->name();
        auto end = n + input_string_length_;
        if (!exact_)
        {
            while (*end != '\0')
                ++end;
        }
        return {n, end};
    }
};

template <>
class found_interface_register<false> : public basic_found_interface_register
{
  public:
    found_interface_register(interface_register* holder, bool /*exact*/ = true)
        : basic_found_interface_register(holder)
    {
    }

    bool exact() const
    {
        ignore_unused(this);
        return true;
    }

    string_view full_name() const
    {
        return name();
    }
};
#endif

interface_register* find_unique(interface_register* first, interface_register* last, char const* name, size_t const length)
{
    interface_register* found;

    if (isdigit(name[length - 1]))
    {
        found = find<false>(first, last, name, length);
    }
    else
    {
        found = find<true>(first, last, name, length);
        if (found != last && found->name()[length] != '\0')
        {
            auto const duplicate = find<true>(found->next(), last, name, length);
            if (duplicate != last)
                return last;
        }
    }

    return found;
}

interface_register* find(interface_register* first, interface_register* last, char const* name, size_t const length)
{
    interface_register* found;

    if (isdigit(name[length - 1]))
        found = find<false>(first, last, name, length);
    else
        found = find<true>(first, last, name, length);

    return found;
}
} // namespace native
}