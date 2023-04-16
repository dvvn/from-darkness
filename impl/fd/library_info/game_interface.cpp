#include <fd/library_info/game_interface.h>

#include <cassert>
#include <cctype>
#include <string_view>
#include <type_traits>

namespace fd
{
enum class game_interface_iterator_mode : uint8_t
{
    normal,
    compare,
    const_compare
};

enum class game_interface_cmp_result : uint8_t
{
    unset,
    error,
    full,
    partial,
};

template <game_interface_iterator_mode Mode>
class game_interface_iterator
{
    using mode = game_interface_iterator_mode;

    using cmp_result = game_interface_cmp_result;
    using cmp_type   = std::conditional_t<Mode == mode::normal, std::false_type, cmp_result>;

    game_interface *current_;
    [[no_unique_address]] cmp_type compared_;

    template <mode>
    friend class game_interface_iterator;

    game_interface_iterator(game_interface *ptr, cmp_type cmp)
        : current_(ptr)
        , compared_(cmp)
    {
    }

  public:
    game_interface_iterator(game_interface_iterator const &) = default;

    template <mode M>
    game_interface_iterator(game_interface_iterator<M> other)
        : current_(other.current_)
        , compared_(other.compared_)
    {
        if constexpr (Mode == mode::const_compare)
            assert(compared_ != cmp_result::error);
    }

    constexpr game_interface_iterator(game_interface *reg)
        : current_(reg)
    {
        if constexpr (Mode != mode::normal)
            compared_ = cmp_result::unset;
    }

    game_interface_iterator &operator++()
    {
        current_ = current_->next;
        return *this;
    }

    game_interface_iterator operator++(int)
    {
        auto tmp = current_;
        current_ = current_->next;
        return tmp;
    }

    game_interface_iterator &operator*() requires(Mode != mode::normal)
    {
        return *this;
    }

    void set(cmp_result status) requires(Mode != mode::normal)
    {
        compared_ = status;
    }

    game_interface const &operator*() const
    {
        return *this->current_;
    }

    game_interface *operator->() const
    {
        return current_;
    }

    game_interface *get() const
    {
        return current_;
    }

    template <mode M>
    bool operator==(game_interface_iterator<M> other) const
    {
        return current_ == other.current_;
    }

    bool operator==(nullptr_t) const
    {
        return !current_;
    }

    explicit operator bool() const
    {
        return current_ != nullptr;
    }

    game_interface_iterator operator+(size_t i) const
    {
        assert(i == 1);
        return { current_->next, compared_ };
    }

    cmp_result status() const
    {
        return compared_;
    }
};

bool operator==(game_interface_iterator<game_interface_iterator_mode::compare> &it, std::string_view interface_name)
{
    auto &curr                 = *std::as_const(it);
    std::string_view curr_name = (curr.name);

    auto cmp = game_interface_cmp_result::error;
    if (curr_name.starts_with(interface_name))
    {
        if (curr_name.size() == interface_name.size())
            cmp = game_interface_cmp_result::full;
        else if (std::isdigit(curr_name[interface_name.size()]))
            cmp = game_interface_cmp_result::partial;
    }
    it.set(cmp);
    return cmp != game_interface_cmp_result::error;
}

bool operator==(
    game_interface_iterator<game_interface_iterator_mode::const_compare> const &it,
    std::string_view interface_name)
{
    switch (it.status())
    {
    case game_interface_cmp_result::full: {
        return it->name == interface_name;
    }
    case game_interface_cmp_result::partial: {
        std::string_view curr_name = (it->name);
        if (curr_name.size() <= interface_name.size())
            return false;
        if (!std::isdigit(curr_name[interface_name.size()]))
            return false;
        return curr_name.starts_with(interface_name);
    }
    default: {
        assert(0 && "Wrong state");
        return false;
    }
    }
}

game_interface game_interface::operator+(size_t offset) const
{
    switch (offset)
    {
    case 0:
        return *this;
    case 1:
        return *next;
    default:
        for (auto src = this;;)
        {
            src = src->next;
            if (--offset == 0)
                return *src;
        }
    }
}

void *game_interface::get() const
{
    return create_fn();
}

game_interface *find_root_game_interface(void *create_func)
{
    assert(create_func != nullptr);

    auto relative_fn  = reinterpret_cast<uintptr_t>(create_func) + 0x5;
    auto displacement = *reinterpret_cast<int32_t *>(relative_fn);
    auto jmp          = relative_fn + sizeof(int32_t) + displacement;

    return **reinterpret_cast<game_interface ***>(jmp + 0x6);
}

static bool _all_digits(char const *ptr)
{
    for (; *ptr != '\0'; ++ptr)
    {
        if (!std::isdigit(*ptr))
            return false;
    }
    return true;
}

found_game_interface find_game_interface(const char *name, size_t length, game_interface *root_interface)
{
    using cmp_result = game_interface_cmp_result;
    using iter_mode  = game_interface_iterator_mode;

    using iterator       = game_interface_iterator<iter_mode::compare>;
    using const_iterator = game_interface_iterator<iter_mode::const_compare>;

    auto target_name = std::string_view(name, length);

    auto target = std::find<iterator>(root_interface, nullptr, target_name);
    assert(target);

    switch (target.status())
    {
    case cmp_result::full:
        return { target.get(), true };
    case cmp_result::partial:
        assert(_all_digits(target->name + length));
        assert(!std::find<const_iterator>(target + 1, nullptr, target_name));
        return { target.get(), false };
    default:
        return { nullptr };
    }
}
} // namespace fd

template <fd::game_interface_iterator_mode Mode>
struct std::iterator_traits<fd::game_interface_iterator<Mode>>
{
    using type       = std::forward_iterator_tag;
    using value_type = fd::game_interface;
};