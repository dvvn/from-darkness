#pragma once

#include <iterator>

namespace fd
{
template <typename T>
class range_view
{
    T begin_, end_;

  public:
    constexpr range_view(T begin, T end)
        : begin_(std::move(begin))
        , end_(std::move(end))
    {
    }

    constexpr T begin() const
    {
        return begin_;
    }

    constexpr T end() const
    {
        return end_;
    }
};

template <typename T>
struct forward_view : range_view<T>
{
    using range_view<std::reverse_iterator<T>>::range_view;

    template <typename Container>
    constexpr forward_view(Container& container)
        : range_view(std::begin(container), std::end(container))
    {
    }
};

template <typename T>
static constexpr void _same_accepter(T, T)
{
}

template <typename T>
concept known_iterator = requires(T obj) { _same_accepter(std::begin(obj), std::end(obj)); };

template <known_iterator T>
struct extract_iterator
{
    using type = decltype(std::begin(std::declval<T>()));
};

template <typename Container>
forward_view(Container& container) -> forward_view<typename extract_iterator<Container>::type>;

template <typename T>
struct reverse_view : range_view<std::reverse_iterator<T>>
{
    using range_view<std::reverse_iterator<T>>::range_view;

    template <typename Container>
    constexpr reverse_view(Container& container)
        : range_view(std::rbegin(container), std::rend(container))
    {
    }
};

template <typename Container>
reverse_view(Container& container) -> reverse_view<typename extract_iterator<Container>::type>;

}