#include "native.h"
#include "pattern.h"
#include "diagnostics/runtime_error.h"
#include "functional/cast.h"
#include "functional/ignore.h"
#include "string/char.h"
#include "string/view.h"

namespace fd
{
template <bool Partial>
class found_native_interface;

class native_interface_holder
{
    void *(*create_)();
    char const *name_;
    native_interface_holder *next_;

  public:
    native_interface_holder() = delete;

    void *get() const
    {
        return create_();
    }

    char const *name() const
    {
        return name_;
    }

    native_interface_holder *next() const
    {
        return next_;
    }
};

static bool all_digits(char const *ptr)
{
    for (auto c = *ptr; c != '\0'; c = *++ptr)
    {
        if (!isdigit(c))
            return false;
    }
    return true;
}

template <bool SkipVersion>
static native_interface_holder const *find(
    native_interface_holder const *first, native_interface_holder const *last, //
    char const *name, size_t const length)
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

class basic_found_native_interface
{
    native_interface_holder *holder_;
    size_t input_string_length_;

    template <bool>
    friend class found_native_interface;

  public:
    // ReSharper disable once CppPossiblyUninitializedMember
    basic_found_native_interface(native_interface_holder *holder)
        : holder_(holder)
    {
    }

    native_interface_holder *operator->() const
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

    basic_found_native_interface &set_input_string_length(size_t const input_string_length)
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
class found_native_interface<true> : public basic_found_native_interface
{
    bool exact_;

  public:
    using basic_found_native_interface::basic_found_native_interface;

    found_native_interface(native_interface_holder *holder, bool const exact)
        : basic_found_native_interface(holder)
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
            end = string_view::traits_type::find(n + input_string_length_, -1, '\0');
        return {n, end};
    }
};

template <>
class found_native_interface<false> : public basic_found_native_interface
{
  public:
    found_native_interface(native_interface_holder *holder, bool /*exact*/ = true)
        : basic_found_native_interface(holder)
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

static native_interface_holder *root_interface(void const *create_fn)
{
    auto const relative_fn  = reinterpret_cast<uintptr_t>(create_fn) + 0x5;
    auto const displacement = *reinterpret_cast<int32_t *>(relative_fn);
    auto const jmp          = relative_fn + sizeof(int32_t) + displacement;

    return **reinterpret_cast<native_interface_holder ***>(jmp + 0x6);
}

void *native_library_info::interface(char const *name, size_t const length) const
{
    auto const root = root_interface(function("CreateInterface"));
    native_interface_holder const *found;
    if (isdigit(name[length - 1]))
    {
        found = find<false>(root, nullptr, name, length);
    }
    else
    {
        found = find<true>(root, nullptr, name, length);
        if (found && found->name()[length] != '\0')
        {
            auto const duplicate = find<true>(found->next(), nullptr, name, length);
            if (duplicate)
                throw runtime_error("Found multiple interfaces for given name");
        }
    }

    return found ? found->get() : nullptr;
}

void *native_library_info::return_address_checker() const
{
    return this->pattern("55 8B EC 56 8B F1 33 C0 57 8B 7D 08 8B 8E"_pat);
}

} // namespace fd