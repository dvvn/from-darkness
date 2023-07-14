#include "native.h"
#include "pattern.h"
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

    bool compare(string_view other_name) const
    {
        return memcmp(name_, other_name.data(), other_name.length());
    }

    void *get() const
    {
        return create_();
    }

    char const *name() const
    {
        return name_;
    }

    template <bool Partial>
    found_native_interface<Partial> find(string_view target_name) const
    {
        for (auto i = remove_const(this); i != nullptr; i = i->next_)
        {
            if constexpr (!Partial)
            {
                if (i->compare(target_name) == 0)
                    return i;
            }
            else
            {
                auto name_back = i->name_ + target_name.length();
                auto exact     = *name_back == '\0';
                if (!exact)
                {
                    for (auto c = name_back; *c != '\0'; ++c)
                    {
                        if (!isdigit(*c))
                            goto _NOT_FOUND;
                    }
                }

                if (i->compare(target_name) == 0)
                    return {i, exact};

            _NOT_FOUND:
                ignore_unused();
            }
        }

        return nullptr;
    }

    native_interface_holder *next() const
    {
        return next_;
    }
};

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

    basic_found_native_interface &set_input_string_length(size_t input_string_length)
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

    found_native_interface(native_interface_holder *holder, bool exact)
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

static native_interface_holder *root_interface(void *create_fn)
{
    auto relative_fn  = reinterpret_cast<uintptr_t>(create_fn) + 0x5;
    auto displacement = *reinterpret_cast<int32_t *>(relative_fn);
    auto jmp          = relative_fn + sizeof(int32_t) + displacement;

    return **reinterpret_cast<native_interface_holder ***>(jmp + 0x6);
}

void *native_library_info::interface(string_view name) const
{
    auto do_find = [root = root_interface(function("CreateInterface")),
                    name]<bool Partial>(std::bool_constant<Partial>) -> void *
    {
        found_native_interface<Partial> found = root->find<Partial>(name);
        found.set_input_string_length(name.length());

        if (!found)
        {
            // log("valve interface {} NOT found", name);
            return nullptr;
        }

        // log("valve interface {} found", found);
        if (!found.exact())
        {
            auto next_interface = found->next();
            if (next_interface)
            {
                auto duplicate = next_interface->template find<true>(name);
                if (duplicate)
                {
                    // log("DUPLICATE valve interface {} found", duplicate);
                    return nullptr;
                }
            }
        }

        return found->get();
    };

    return isdigit(name.back()) ? do_find(std::false_type()) : do_find(std::true_type());
}

auto native_library_info::return_address_checker() const -> return_address_checker_t
{
    auto ptr = this->pattern("55 8B EC 56 8B F1 33 C0 57 8B 7D 08 8B 8E"_pat);
    return unsafe_cast<return_address_checker_t>(ptr);
}

} // namespace fd