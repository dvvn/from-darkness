#pragma once

#include <iterator>

namespace fd
{
template <class It>
constexpr decltype(auto) decay_iter(It it)
{
    if constexpr (std::_Unwrappable_v<It>)
        return std::_Get_unwrapped(it);
    else
        return it;
}

template <class T>
concept have_unchecked = requires(T& obj) {
                             obj._Unchecked_begin();
                             obj._Unchecked_end();
                         };

template <class T>
concept have_size = requires(T& obj) { std::size(obj); };

template <class T>
concept have_data = requires(T& obj) {
                        std::data(obj);
                        std::size(obj);
                    };

template <typename T>
concept have_begin_end = requires(T container) {
                             std::begin(container);
                             std::end(container);
                         };

template <typename T>
static constexpr auto _begin(T& container)
{
    if constexpr (have_unchecked<T>)
        return container._Unchecked_begin();
    else if constexpr (have_data<T>)
        return std::data(container);
    else if constexpr (have_begin_end<T>)
        return decay_iter(std::begin(container));
}

template <typename T>
static constexpr auto _end(T& container)
{
    if constexpr (have_unchecked<T>)
        return container._Unchecked_end();
    else if constexpr (have_data<T>)
        return std::data(container) + std::size(container);
    else if constexpr (have_begin_end<T>)
        return decay_iter(std::end(container));
}

template <typename T>
static constexpr size_t _size(T&& container)
{
    if constexpr (have_size<T>)
        return std::size(container);
    else if constexpr (have_unchecked<T>)
        return std::distance(container._Unchecked_begin(), container._Unchecked_end());
    else if constexpr (have_begin_end<T>)
        return std::distance(_unwrap(std::begin(container), decay_iter(std::end(container))));
}

template <typename T>
static constexpr auto _size_or_end(T& container)
{
    if constexpr (have_size<T>)
        return std::size(container);
    else if constexpr (have_unchecked<T>)
        return container._Unchecked_end();
    else if constexpr (have_begin_end<T>)
        return decay_iter(std::end(container));
}

template <typename T>
concept native_iterable = have_unchecked<T> || have_data<T> || have_begin_end<T>;

template <typename T>
using begin_t = decltype(_begin(std::declval<T&>()));

template <typename T>
using end_t = decltype(_end(std::declval<T&>()));

template <typename T>
class range_view;

template <typename T>
class range_view
{
    T begin_, end_;

    friend class range_view<const T>;
    friend class range_view<std::remove_const_t<T>>;

  public:
    constexpr range_view(T begin, T end)
        : begin_(std::move(begin))
        , end_(std::move(end))
    {
    }

    constexpr range_view(T begin, size_t size)
        // ReSharper disable once CppMemberInitializersOrder
        : end_(begin + size)
        , begin_(std::move(begin))

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

    constexpr bool empty() const
    {
        return begin_ == end_;
    }

    constexpr range_view subrange(const size_t offset) const
    {
        return { begin_ + offset, end_ };
    }

    constexpr range_view subrange(const T newBegin) const
    {
        return { newBegin, end_ };
    }

    constexpr auto distance(const T end) const
    {
        return std::distance(begin_, end);
    }
};

template <class T>
class forward_view_lazy
{
    T* source_;

  public:
    constexpr forward_view_lazy(T& source)
        : source_(&source)
    {
    }

    constexpr decltype(auto) begin() const
    {
        return _begin(*source_);
    }

    constexpr decltype(auto) end() const
    {
        return _end(*source_);
    }

    constexpr size_t size() const
    {
        return _size(*source_);
    }

    constexpr decltype(auto) size_or_end() const
    {
        return _size_or_end(*source_);
    }
};

template <class T>
class reverse_view_lazy
{
    T* source_;

  public:
    constexpr reverse_view_lazy(T& source)
        : source_(&source)
    {
    }

    constexpr auto begin() const
    {
        return std::reverse_iterator(_end(*source_));
    }

    constexpr auto end() const
    {
        return std::reverse_iterator(_begin(*source_));
    }
};

template <typename Container>
constexpr auto forward_view(Container& container)
{
    return range_view(_begin(container), _end(container));
}

template <typename Container>
constexpr auto reverse_view(Container& container)
{
    return range_view(std::reverse_iterator(_end(container)), std::reverse_iterator(_begin(container)));
}
} // namespace fd