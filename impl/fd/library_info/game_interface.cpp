#include "game_interface.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <type_traits>

namespace fd
{
class game_interface_iterator
{
    game_interface *current_;

  public:
    // using iterator_concept  = std::contiguous_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type        = game_interface;
    using difference_type   = ptrdiff_t;
    using pointer           = game_interface *;
    using reference         = game_interface &;

    game_interface_iterator()
    {
    }

    constexpr game_interface_iterator(game_interface *current)
        : current_(current)
    {
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

    game_interface &operator*() const
    {
        return *this->current_;
    }

    game_interface *operator->() const
    {
        return current_;
    }

    operator game_interface *() const
    {
        return current_;
    }

    bool operator==(game_interface_iterator other) const
    {
        return current_ == other.current_;
    }

    bool operator==(game_interface *) const = delete;
};

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

game_interface *find_game_interface(game_interface *root_interface, const char *name, size_t length, bool exact)
{
    // unsafe comparison, but ok

    using iterator = game_interface_iterator;

    iterator target;

    auto compare_partial = [&](game_interface &ifc) {
        return (ifc.name[length] == '\0' || isdigit(ifc.name[length])) && std::memcmp(name, ifc.name, length) == 0;
    };
    auto compare_exact = [&](game_interface &ifc) {
        return ifc.name[length] == '\0' && std::memcmp(name, ifc.name, length) == 0;
    };

    if (exact)
    {
        target = std::find_if<iterator>(root_interface, nullptr, compare_exact);
    }
    else
    {
        target = std::find_if<iterator>(root_interface, (nullptr), compare_partial);

#ifdef _DEBUG
        if (target && target->name[length] != '\0')
        {
            assert(_all_digits(target->name + length + 1));
            assert(!std::find_if<iterator>(std::next(target), nullptr, compare_partial));
        }
#endif
    }

    return target;

#if 0
    switch (target.status())
    {
    case cmp_result::full:
        return {target.get(), true};
    case cmp_result::partial:
        assert(_all_digits(target->name + length));
        assert(!std::find<const_iterator>(target + 1, nullptr, target_name));
        return {target.get(), false};
    default:
        return {nullptr};
    }
#endif
}
} // namespace fd