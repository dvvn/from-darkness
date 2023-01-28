#pragma once

#include <iterator>

namespace fd
{
template <typename T>
class range_view;

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

    constexpr range_view(T begin, size_t size)
        : begin_(begin)
        , end_(std::next(begin, size))
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

    constexpr size_t size() const
    {
        return std::distance(begin_, end_);
    }
};

template <typename Container>
constexpr auto forward_view(Container& container) -> range_view<decltype(std::begin(container))>
{
    return { std::begin(container), std::end(container) };
}

template <typename Container>
constexpr auto reverse_view(Container& container) -> range_view<decltype(std::rbegin(container))>
{
    return { std::rbegin(container), std::rend(container) };
}
} // namespace fd