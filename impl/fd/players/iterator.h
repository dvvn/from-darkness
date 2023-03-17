#pragma once

#include <fd/players/filter.h>
#include <fd/players/player.h>

#include <concepts>

namespace fd
{
template <typename It>
class players_iterator
{
    It it_;
    players_filter filter_;

    template <typename It2>
    friend class players_iterator;

  public:
    // #ifdef __cpp_lib_concepts
    //    using iterator_concept = typename std::iterator_traits<It>::iterator_concept;
    // #endif
    //    using iterator_category = typename std::iterator_traits<It>::random_access_iterator_tag;
    //    using value_type        = typename std::iterator_traits<It>::value_type;
    //    using difference_type   = typename std::iterator_traits<It>::difference_type;
    //    using pointer           = typename std::iterator_traits<It>::pointer;
    //    using reference         = typename std::iterator_traits<It>::reference;

    players_iterator() = default;

    players_iterator(std::constructible_from<It> auto it, players_filter filter)
        : it_(std::move(it))
        , filter_(std::move(filter))
    {
    }

    players_iterator(std::constructible_from<It> auto it) // for end
        : it_(std::move(it))
    {
    }

    players_iterator &operator++()
    {
        (++it_);
        return *this;
    }

    players_iterator operator++(int)
    {
        auto ret = *this;
        (++it_);
        return ret;
    }

    player *operator*()
    {
        filter_(it_);
        return *it_;
    }

    player *operator*() const
    {
        return *it_;
    }

    /*bool operator==(players_iterator const &other) const
    {
        return it_ == other.it_;
    }*/

    template <std::equality_comparable_with<It> It2>
    bool operator==(players_iterator<It2> const &other) const
    {
        return it_ == other.it_;
    }

    template <std::equality_comparable_with<It> It2>
    bool operator==(It2 other) const // requires(sizeof(It) == sizeof(It2))
    {
        return it_ == other;
    }
};
} // namespace fd

template <typename It>
struct std::iterator_traits<fd::players_iterator<It>> : iterator_traits<It>
{
};