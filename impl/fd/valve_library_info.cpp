#include "log.h"
#include "tool/functional.h"
#include "tool/string_view.h"
#include "tool/vector.h"
#include "valve_library_info.h"

#include <cctype>

static bool isdigit(char c)
{
    // locale's anywway unused
    return c >= '0' && c <= '9';
}

namespace fd
{
class basic_found_valve_interface;
}

template <std::derived_from<fd::basic_found_valve_interface> T, typename C>
struct fmt::formatter<T, C> : formatter<basic_string_view<C>, C>
{
    using base = formatter<basic_string_view<C>, C>;

    auto format(T const &valve_interface, auto &ctx) const -> decltype(ctx.out())
    {
        fd::small_vector<C, 64> buff;
        auto name = valve_interface.name();

        if (!valve_interface.exact())
            format_to(std::back_inserter(buff), "{} ({})", name, valve_interface.full_name());
        else
        {
            if constexpr (std::same_as<C, char>)
                return base::format({name.data(), name.size()}, ctx);
            else
                buff.assign(name.begin(), name.end());
        }
        return base::format({buff.data(), buff.size()}, ctx);
    }
};

namespace fd
{
template <bool Partial>
class found_valve_interface;

class valve_interface_holder
{
    using pointer = valve_interface_holder const *;

    void *(*create_)();
    char const *name_;
    pointer next_;

  public:
    valve_interface_holder() = delete;

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
    found_valve_interface<Partial> find(string_view target_name) const
    {
        for (auto i = this; i != nullptr; i = i->next_)
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

    pointer next() const
    {
        return next_;
    }
};

class basic_found_valve_interface
{
    using pointer = valve_interface_holder const *;

    pointer holder_;
    size_t input_string_length_;

    template <bool>
    friend class found_valve_interface;

  public:
    // ReSharper disable once CppPossiblyUninitializedMember
    basic_found_valve_interface(pointer holder)
        : holder_(holder)
    {
    }

    pointer operator->() const
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

    basic_found_valve_interface &set_input_string_length(size_t input_string_length)
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
class found_valve_interface<true> : public basic_found_valve_interface
{
    bool exact_;

  public:
    using basic_found_valve_interface::basic_found_valve_interface;

    found_valve_interface(pointer holder, bool exact)
        : basic_found_valve_interface(holder)
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
class found_valve_interface<false> : public basic_found_valve_interface
{
  public:
    found_valve_interface(pointer holder, bool /*exact*/ = true)
        : basic_found_valve_interface(holder)
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

static valve_interface_holder *root_interface(void *create_fn)
{
    auto relative_fn  = reinterpret_cast<uintptr_t>(create_fn) + 0x5;
    auto displacement = *reinterpret_cast<int32_t *>(relative_fn);
    auto jmp          = relative_fn + sizeof(int32_t) + displacement;

    return **reinterpret_cast<valve_interface_holder ***>(jmp + 0x6);
}

void *valve_library::interface(string_view name) const
{
    auto do_find =
        [=, root = root_interface(function("CreateInterface"))]<bool Partial>(std::bool_constant<Partial>) -> void * {
        found_valve_interface<Partial> found = root->find<Partial>(name);
        found.set_input_string_length(name.length());

        if (!found)
        {
            log("valve interface {} NOT found", name);
            return nullptr;
        }

        log("valve interface {} found", found);
        if (!found.exact())
        {
            auto next_interface = found->next();
            if (next_interface)
            {
                auto duplicate = next_interface->template find<true>(name);
                if (duplicate)
                {
                    log("DUPLICATE valve interface {} found", duplicate);
                    return nullptr;
                }
            }
        }

        return found->get();
    };

    return isdigit(name.back()) ? do_find(std::false_type()) : do_find(std::true_type());
}
} // namespace fd