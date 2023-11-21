#include "native/interface_register.h"
#include "algorithm/char.h"
#include "iterator/unwrap.h"
#include "string/view.h"

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

interface_register* interface_register::next() const
{
    return next_;
}

enum class find_helper_flags : uint8_t
{
    unused,
    whole_check,
    duplicate_check
};

template <typename Last>
class find_helper
{
    static constexpr auto check_first_char_table = [] {
        auto table                         = detail::isdigit_table;
        detail::char_table_get(table, '_') = true;
        return table;
    }();

    template <bool KnowVersion>
    static bool check_first_version_char(char const* name_last_char, std::bool_constant<KnowVersion>)
    {
        if constexpr (KnowVersion)
            return *name_last_char == '\0';
        else
            return detail::char_table_get(check_first_char_table, *name_last_char);
    }

    template <bool KnowVersion>
    static auto check_version(char const* str, std::bool_constant<KnowVersion> ver)
    {
        if constexpr (!KnowVersion)
            for (;;)
            {
                if (check_first_version_char(str, ver))
                    continue;

                if (*++str == '\0')
                    break;

                return false;
            }
        return true;
    }

    interface_register* first_;
    [[no_unique_address]] //
    Last last_;
    char const* name_;
    size_t const name_length_;

    find_helper next(interface_register* next) const
    {
        if constexpr (!std::is_null_pointer_v<Last>)
            verify_range(first_, last_);
        return {next, last_, name_, name_length_};
    }

  public:
    find_helper(interface_register* first, Last last, char const* name, size_t const name_length)
        : first_{first}
        , last_{last}
        , name_{name}
        , name_length_{name_length}
    {
    }

    template <bool KnowVersion>
    interface_register* get(std::bool_constant<KnowVersion> ver)
    {
        for (; first_ != last_; first_ = first_->next())
        {
            auto const src_name = first_->name();

            if (!check_first_version_char(src_name + name_length_, ver))
                continue;
            if (memcmp(src_name, name_, name_length_) != 0)
                continue;
            if (!check_version(src_name + name_length_ + 1, ver))
                continue;

            break;
        }
        return first_;
    }

    template <find_helper_flags Flag>
    interface_register* get(std::integral_constant<find_helper_flags, Flag> = {})
    {
        constexpr auto know_version   = std::true_type();
        constexpr auto unknow_version = std::false_type();

        if constexpr (Flag != find_helper_flags::whole_check)
        {
            if (isdigit(name_[name_length_ - 1]))
                return get(know_version);
        }

        auto const found = get(unknow_version);

        if constexpr (Flag == find_helper_flags::duplicate_check)
            if (found != last_ && found->name()[name_length_] != '\0')
            {
                auto const duplicate = next(found->next()).get(unknow_version);
                if (duplicate != last_)
                    return last_;
            }

        return found;
    }
};

interface_register* find_unique(
    interface_register* first, interface_register* last, //
    char const* name, size_t const name_length)
{
    return find_helper(first, last, name, name_length).get<find_helper_flags::duplicate_check>();
}

interface_register* find(
    interface_register* first, interface_register* last, //
    char const* name, size_t const name_length)
{
#if defined(_DEBUG) && 0
    constexpr auto flag = find_helper_flags::duplicate_check;
#else
    constexpr auto flag = find_helper_flags::unused;
#endif
    return find_helper(first, last, name, name_length).get<flag>();
}
}